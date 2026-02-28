#define TB_IMPL
#include <notmuch.h>
#include "termbox2.h"
#include "config.h"

#define MAX_THREADS 100
#define MAX_MSGS_PER_THREAD 256

#define Q(name, label, query) [Q_##name] = query,
static const char *query_str[Q_COUNT] = { QUERY_LIST };
#undef Q

#define Q(name, label, query) [Q_##name] = label,
static const char *query_label[Q_COUNT] = { QUERY_LIST };
#undef Q

struct state {
    int width;
    int height;
    int sel;
    int query_id;
    int thread_count;
    int msg_count;
    int msg_unread;
};

struct msg {
    char *id;
    char *date;
    char *from;
    char *subj;
    char *tags;
    int depth;
    int last_sibling;
    int has_children;
};

struct thread {
    char *id;
    int msgs;
    int unfolded;
    struct msg msg[MAX_MSGS_PER_THREAD];
};

static struct state state = {0};
static struct thread threads[MAX_THREADS] = {0};
static int ret;
static notmuch_database_t *db = NULL;

static void end(const char *msg);
static int utf8_width(const char *str);
static void free_threads(void);
static char *parse_from(const char *from);
static void nm_query(void);
static void render_select(void);
static void render(void);

static void end(const char *msg) {
    FILE *logfile = fopen("/tmp/neomuch.log", "w");
    if (logfile) {
        fprintf(logfile, "%s: %d\n", msg, ret);
        fflush(logfile);
        fclose(logfile);
    }
    notmuch_database_destroy(db);
    tb_shutdown();
    exit(ret);
}

static int utf8_width(const char *str) {
    if (!str) return 0;
    int cols = 0;
    while (*str) {
        unsigned char c = (unsigned char)*str;
        if (c < 0x80) { cols++; str++; }
        else if ((c & 0xE0) == 0xC0) { cols++; str += 2; }
        else if ((c & 0xF0) == 0xE0) { cols++; str += 3; }
        else if ((c & 0xF8) == 0xF0) { cols++; str += 4; }
        else { str++; }
    }
    return cols;
}

static void free_threads(void) {
    for (int i = 0; i < state.thread_count; i++) {
        free(threads[i].id);
        for (int j = 0; j < threads[i].msgs; j++) {
            free(threads[i].msg[j].id);
            free(threads[i].msg[j].date);
            free(threads[i].msg[j].from);
            free(threads[i].msg[j].subj);
            free(threads[i].msg[j].tags);
        }
    }
    state.thread_count = 0;
}

static char *parse_from(const char *from) {
    if (!from) return strdup("?");

    const char *start = from;
    while (*start == '\"') start++;

    const char *end = strchr(start, '<');
    size_t len = end ? (size_t)(end - start) : strlen(start);

    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\"'))
        len--;

    if (len == 0) {
        // fallback: use original header as-is
        return strdup(from);
    }

    char *name = malloc(len + 1);
    if (!name) return strdup(from);  // fallback on malloc failure

    strncpy(name, start, len);
    name[len] = '\0';

    return name;
}

static void nm_query(void) {
    free_threads();
    int count = 0;
    int total_msgs = 0;

    notmuch_query_t *q = notmuch_query_create(db, query_str[state.query_id]);
    notmuch_query_set_sort(q, NOTMUCH_SORT_NEWEST_FIRST);

    notmuch_threads_t *iter = NULL;
    if (notmuch_query_search_threads(q, &iter) != NOTMUCH_STATUS_SUCCESS || !iter) {
        notmuch_query_destroy(q);
        state.thread_count = 0;
        return;
    }

    while (notmuch_threads_valid(iter) && count < MAX_THREADS) {
        notmuch_thread_t *thread = notmuch_threads_get(iter);
        if (!thread) { notmuch_threads_move_to_next(iter); continue; }

        threads[count].id = strdup(notmuch_thread_get_thread_id(thread));
        threads[count].msgs = 0;
        threads[count].unfolded = 0;

        notmuch_messages_t *allmsgs = notmuch_thread_get_messages(thread);
        if (allmsgs) {
            int m_idx = 0;
            while (notmuch_messages_valid(allmsgs) && m_idx < MAX_MSGS_PER_THREAD) {
                notmuch_message_t *m = notmuch_messages_get(allmsgs);
                if (!m) { notmuch_messages_move_to_next(allmsgs); continue; }

                struct msg *dst = &threads[count].msg[m_idx];

                const char *mid = notmuch_message_get_message_id(m);
                dst->id = strdup(mid ? mid : "");

                time_t mt = notmuch_message_get_date(m);
                char dbuf[32] = "??? ??";
                if (mt > 0) {
                    struct tm *tm_info = localtime(&mt);
                    if (tm_info) strftime(dbuf, sizeof dbuf, "%b %d", tm_info);
                }
                dst->date = strdup(dbuf);

                const char *mf = notmuch_message_get_header(m, "from");
                dst->from = parse_from(mf);

                const char *msub = notmuch_message_get_header(m, "subject");
                dst->subj = strdup(msub && *msub ? msub : "(no subject)");

                char tbuf[256] = "";
                notmuch_tags_t *mtags = notmuch_message_get_tags(m);
                if (mtags) {
                    int first = 1;
                    while (notmuch_tags_valid(mtags)) {
                        const char *tag = notmuch_tags_get(mtags);
                        if (tag && *tag) {
                            if (!first) strncat(tbuf, " ", sizeof(tbuf)-strlen(tbuf)-1);
                            strncat(tbuf, tag, sizeof(tbuf)-strlen(tbuf)-1);
                            first = 0;
                        }
                        notmuch_tags_move_to_next(mtags);
                    }
                    notmuch_tags_destroy(mtags);
                }
                dst->tags = strdup(tbuf);

                dst->depth = 0;
                dst->last_sibling = 0;
                dst->has_children = 0;

                m_idx++;
                notmuch_messages_move_to_next(allmsgs);
            }
            notmuch_messages_destroy(allmsgs);
            threads[count].msgs = m_idx;
            total_msgs += m_idx;
        }

        notmuch_thread_destroy(thread);
        notmuch_threads_move_to_next(iter);
        count++;
    }

    notmuch_threads_destroy(iter);
    notmuch_query_destroy(q);

    state.thread_count = count;
    state.msg_count = total_msgs;
    state.msg_unread = 0; // still unused
}

static void render_select(void) {
    int y = state.sel + 1;
    for (int x = 0; x < state.width; x++) {
        struct tb_cell *cell = NULL;
        if (tb_get_cell(x, y, 1, &cell) == 0 && cell) {
            tb_set_cell(x, y, cell->ch, cell->fg | DEC_SEL_FG, cell->bg | DEC_SEL_BG);
        }
    }
    tb_present();
}

static void render(void) {
    tb_clear();
    tb_printf(0, 0, DEC_STAT, FMT_STAT);
    int y = 1;

    for (int i = 0; i < state.thread_count; i++) {
        struct thread *t = &threads[i];
        for (int j = 0; j < t->msgs; j++) {
            struct msg *m = &t->msg[j];
            int x = 0;
            tb_printf(x, y, DEC_DATE, FMT_DATE); x += FMT_DATE_WIDTH;
            tb_printf(x, y, DEC_MSGS, FMT_MSGS); x += FMT_MSGS_WIDTH;
            tb_printf(x, y, DEC_FROM, FMT_FROM); x += FMT_FROM_WIDTH;
            tb_printf(x, y, DEC_SUBJ, FMT_SUBJ); x += FMT_SUBJ_WIDTH;
            tb_printf(x, y, DEC_TAGS, FMT_TAGS);
            y++;
        }
    }
    render_select();
}

int main(void) {
    struct tb_event ev;

    ret = notmuch_database_open_with_config(
        NOTMUCH_DATABASE_PATH,
        NOTMUCH_DATABASE_MODE_READ_ONLY, NULL, NULL, &db, NULL);
    if (ret != 0) end("notmuch_database_open_with_config");

    ret = tb_init();
    if (ret != 0) end("tb_init");

    tb_set_input_mode(TB_INPUT_ESC);

    state.width = tb_width();
    state.height = tb_height();
    nm_query();

    while (1) {
        render();
        tb_poll_event(&ev);

        if (ev.type == TB_EVENT_KEY) {
            switch (ev.ch) {
                case KEY_QUIT: ret = 0; end("KEY_QUIT"); break;
                case KEY_UP: if (state.sel > 0) state.sel--; break;
                case KEY_DOWN: if (state.sel < state.thread_count - 1) state.sel++; break;
                case KEY_QUERY_NEXT: state.query_id = (state.query_id + 1) % Q_COUNT; nm_query(); break;
                case KEY_QUERY_PREV: state.query_id = (state.query_id - 1 + Q_COUNT) % Q_COUNT; nm_query(); break;
            }
        } else if (ev.type == TB_EVENT_RESIZE) {
            state.width = tb_width();
            state.height = tb_height();
        }
    }
}
