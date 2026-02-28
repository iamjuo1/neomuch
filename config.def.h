#define NOTMUCH_DATABASE_PATH NULL

// QUERIES aka MAILBOXES aka VIRTUAL MAILBOXES
#define QUERY_LIST                                       \
	Q(INBOX,  "INBOX",   "tag:inbox")                    \
	Q(ARCHIV, "ARCHIVE", "tag:archive")                  \
	Q(ATTCH,  "ATTCH",   "tag:attachment")               \
	Q(IBKR,   "IBKR",    "from:@interactivebrokers.com") \
	Q(SENT,   "SENT",    "from:@iamjuo.com")             \
	Q(ALL,    "*",       "*")

#define Q(name, label, query) Q_##name,
enum { QUERY_LIST Q_COUNT };
#undef Q

// KEYS
#define KEY_QUIT       'q'
#define KEY_UP         'w'
#define KEY_DOWN       's'
#define KEY_BACK       'a'
#define KEY_ACTION     'd'
#define KEY_QUERY_NEXT 'n'
#define KEY_QUERY_PREV 'p'
#define KEY_COMPOSE    'm'
#define KEY_REPLY      'r'
#define KEY_UNFOLD     TB_KEY_SPACE

// DECORATIONS, read termbox2.h for ideas
#define DEC_DATE   TB_MAGENTA,               TB_DEFAULT
#define DEC_MSGS   TB_MAGENTA,               TB_DEFAULT
#define DEC_FROM   TB_YELLOW,                TB_DEFAULT
#define DEC_SUBJ   TB_BLUE,                  TB_DEFAULT
#define DEC_TAGS   TB_MAGENTA,               TB_DEFAULT
#define DEC_STAT   ( TB_DEFAULT | TB_BOLD ), TB_DEFAULT
#define DEC_SEL_FG TB_UNDERLINE
#define DEC_SEL_BG TB_DEFAULT

// FORMAT
#define FMT_STAT " NeoMuch: %s %d/%d", query_label[state.query_id], state.msg_unread, state.msg_count
#define FMT_DATE " %-6s",   threads[i].date
#define FMT_MSGS " [1/%d]", threads[i].total_msgs
#define FMT_FROM " %.18s",  threads[i].from
#define FMT_SUBJ " %s",     threads[i].subject
#define FMT_TAGS " # %s",   threads[i].tags
#define FMT_DATE_WIDTH 1 + 6
#define FMT_MSGS_WIDTH 5 +      snprintf(buf, sizeof buf, "%d", threads[i].total_msgs)
#define FMT_FROM_WIDTH 1 + 18 - snprintf(buf, sizeof buf, "%d", threads[i].total_msgs)
#define FMT_SUBJ_WIDTH 1 + utf8_width(threads[i].subject)
