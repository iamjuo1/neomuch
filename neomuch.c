#define TB_IMPL
#include <notmuch.h>
#include "config.h"
#include "termbox2.h"

#define Q(name, label, query) [Q_##name] = query,
static const char *qstr[Q_COUNT] = { QUERY_LIST };
#undef Q

#define Q(name, label, query) [Q_##name] = label,
static const char *qlabel[Q_COUNT] = { QUERY_LIST };
#undef Q

struct msg
{
	notmuch_message_t *m;
	const char *id;
	time_t date;
	const char *from;
	const char *subj;
	const char *tags;
};

struct thread
{
	notmuch_thread_t *t;
	int msgs;
	int unfold;
	struct msg msg[MAX_MSGS_PER_THREAD];
};

static struct thread threads[MAX_THREADS] = {0};
static notmuch_database_t *db = NULL;
static notmuch_threads_t *thread_iter = NULL;
static notmuch_query_t *q = NULL;

static int ret  = 0;
static int w    = 0;
static int h    = 0;
static int sel  = 0;
static int qid  = 0;
static int rows = 0;
static unsigned int msg_count  = 0;
static unsigned int msg_unread = 0;
static unsigned int thr_count  = 0;
static unsigned int page       = 0;
static unsigned int page_size  = 0;

static void end(const char *msg);
static int utf8_width(const char *str);
static const char *fmt_date(time_t t);
static const char *fmt_from(const char *from);
static const char *fmt_tree(int i, int j);
static void nm_query(void);
static void render_select(void);
static void render(void);
static void unfold(void);

static void end(const char *msg)
{
	FILE *logfile = fopen("/tmp/neomuch.log", "w");
	if (logfile)
	{
		fprintf(logfile, "%s: %d\n", msg, ret);
		fclose(logfile);
	}
	if (db) notmuch_database_destroy(db);
	tb_shutdown();
	exit(ret);
}

static int utf8_width(const char *str)
{
	if (!str) return 0;
	int cols = 0;
	while (*str)
	{
		unsigned char c = (unsigned char) * str;
		if (c < 0x80) { cols++; str++; }
		else if ((c & 0xE0) == 0xC0) { cols++; str += 2; }
		else if ((c & 0xF0) == 0xE0) { cols++; str += 3; }
		else if ((c & 0xF8) == 0xF0) { cols++; str += 4; }
		else { str++; }
	}
	return cols;
}

static const char *fmt_date(time_t t)
{
	static char buf[16];
	time_t stamp = time(NULL);
	struct tm now = *localtime(&stamp);
	struct tm msg = *localtime(&t);
	if (msg.tm_year == now.tm_year && msg.tm_yday == now.tm_yday)
		strftime(buf, sizeof(buf), STRFTIME_TODAY, &msg);
	else
		strftime(buf, sizeof(buf), STRFTIME_BASE, &msg);
	return buf;
}

static const char *fmt_from(const char *from)
{
	while (*from == '"') from++;

	// find first '<' or end of string
	const char *end = strchr(from, '<');
	size_t len = end ? (size_t)(end - from) : strlen(from);

	// trim trailing spaces/quotes
	while (len > 0 && (from[len - 1] == ' ' || from[len - 1] == '"'))
		len--;

	if (len == 0) return from;

	static char buf[128];
	if (len >= sizeof(buf)) len = sizeof(buf) - 1;
	memcpy(buf, from, len);
	buf[len] = '\0';
	return buf;
}

static const char *fmt_tree(int i, int j)
{
	if (j == 0) return "";
	return " ├─>";
}

static void nm_query(void)
{
	if (q) { notmuch_query_destroy(q); q = NULL; }
	if (qid < 0) qid = 0; else if (qid >= Q_COUNT) qid = Q_COUNT - 1;

	q = notmuch_query_create(db, qstr[qid]);
	notmuch_query_set_sort(q, NOTMUCH_SORT_NEWEST_FIRST);
	notmuch_query_count_messages(q, &msg_count);
	notmuch_query_count_threads(q, &thr_count);
	if (thr_count > MAX_THREADS) thr_count = MAX_THREADS;

	ret = notmuch_query_search_threads(q, &thread_iter); if (ret != 0) return;

	unsigned int i = 0;

	while (notmuch_threads_valid(thread_iter) && i < thr_count)
	{
		notmuch_thread_t *t = notmuch_threads_get(thread_iter);
		if (!t) { notmuch_threads_move_to_next(thread_iter); continue; }

		threads[i].t = t;
		threads[i].msgs = notmuch_thread_get_matched_messages(t);
		threads[i].unfold = 0;

		notmuch_messages_t *allmsgs = notmuch_thread_get_messages(t);
		int m_idx = 0;

		while (allmsgs && notmuch_messages_valid(allmsgs) && m_idx < MAX_MSGS_PER_THREAD)
		{
			notmuch_message_t *m = notmuch_messages_get(allmsgs);
			if (!m) { notmuch_messages_move_to_next(allmsgs); continue; }

			struct msg *dst = &threads[i].msg[m_idx];
			dst->m    = m;
			dst->id   = notmuch_message_get_message_id(m);
			dst->from = notmuch_message_get_header(m, "from");
			dst->subj = notmuch_message_get_header(m, "subject");
			dst->date = notmuch_message_get_date(m);

			// Tags: collect in static buffer
			static char tbuf[256];
			tbuf[0] = '\0';
			notmuch_tags_t *mtags = notmuch_message_get_tags(m);
			if (mtags)
			{
				int first = 1;
				while (notmuch_tags_valid(mtags))
				{
					const char *tag = notmuch_tags_get(mtags);
					if (!first) strncat(tbuf, " ", sizeof(tbuf) - strlen(tbuf) -1);
					strncat(tbuf, tag, sizeof(tbuf) - strlen(tbuf) -1);
					first = 0;
					notmuch_tags_move_to_next(mtags);
				}
				notmuch_tags_destroy(mtags);
			}
			dst->tags = tbuf;

			m_idx++;
			notmuch_messages_move_to_next(allmsgs);
		}
		if (allmsgs) notmuch_messages_destroy(allmsgs);

		i++;

		notmuch_threads_move_to_next(thread_iter);
	}
}

static void unfold(void)
{
	int range = 1;
	unsigned int i;
	for (i = 0; i < thr_count; i++)
	{
		if (threads[i].unfold)
		{
			if (range <= sel && sel < range + threads[i].msgs) break;
			else range += threads[i].msgs;
		}
		else { if (sel == range) break; else range++; }
	}
	sel = range;
	threads[i].unfold ^= 1;
}

static void render_select(void)
{
	if (thr_count == 0) { sel = 0; return; }
	int y = sel - page * page_size;
	for (int x = 0; x < w; x++)
	{
		struct tb_cell *cell = NULL;
		if (tb_get_cell(x, y, 1, &cell) == 0 && cell)
			tb_set_cell(x, y, cell->ch,
			            cell->fg | DEC_SEL_FG,
			            cell->bg | DEC_SEL_BG);
	}
}

static void render(void)
{
	tb_clear();
	if (sel < 1) sel = 1; else if (sel > rows) sel = rows;
	page = (sel - 1) / page_size;
	rows = 0;
	int y = 0;
	for (unsigned int i = 0; i < thr_count; i++)
	{
		struct thread *t = &threads[i];
		for (int j = 0; j < t->msgs; j++)
		{
			rows++; y = rows - page * page_size;
			if (y == 0) y++;
			struct msg *m = &t->msg[j];
			const char *date = fmt_date(m->date);
			const char *from = fmt_from(m->from);
			const char *tree = fmt_tree(i, j);
			const char *subj = m->subj;
			if (t->unfold && j > 0) subj = NULL;
			int x = 0;
			tb_printf(x, y, DEC_DATE, FMT_DATE); x += FMT_DATE_WIDTH;
			tb_printf(x, y, DEC_MSGS, FMT_MSGS); x += FMT_MSGS_WIDTH;
			tb_printf(x, y, DEC_FROM, FMT_FROM); x += FMT_FROM_WIDTH;
			tb_printf(x, y, DEC_TREE, FMT_TREE); x += FMT_TREE_WIDTH;
			tb_printf(x, y, DEC_SUBJ, FMT_SUBJ); x += FMT_SUBJ_WIDTH;
			tb_printf(x, y, DEC_TAGS, FMT_TAGS);
			if (!t->unfold) break;
		}
	}
	render_select();
	tb_printf(0, 0, DEC_STAT, FMT_STAT);
	tb_present();
}

int main(void)
{
	struct tb_event ev;

	ret = notmuch_database_open_with_config(
	          NOTMUCH_DATABASE_PATH,
	          NOTMUCH_DATABASE_MODE_READ_ONLY,
	          NULL, NULL, &db, NULL);
	if (ret != 0) end("notmuch_database_open_with_config");

	ret = tb_init(); if (ret != 0) end("tb_init");

	tb_set_input_mode(TB_INPUT_ESC);
	tb_set_output_mode(COL_MODE);

	w = tb_width();
	h = tb_height();
	page_size = h - 1;

	nm_query();

	while (1)
	{
		render();
		tb_poll_event(&ev);

		if (ev.type == TB_EVENT_KEY)
		{
			switch (ev.ch)
			{
			case KEY_QUIT:   ret = 0; end("KEY_QUIT"); break;
			case KEY_UP:     sel--; break;
			case KEY_DOWN:   sel++; break;
			case KEY_TOP:    sel = 0; break;
			case KEY_BOT:    sel = rows; break;
			case KEY_Q_NEXT: qid++; nm_query(); break;
			case KEY_Q_PREV: qid--; nm_query(); break;
			case KEY_UNFOLD: unfold(); break;
			}
		}
		else if (ev.type == TB_EVENT_RESIZE)
		{
			w = tb_width();
			h = tb_height();
			page_size = h - 1;
		}
	}
}
