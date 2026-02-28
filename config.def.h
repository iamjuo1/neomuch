// Keep NULL to use value from Notmuch config or ENV
#define NOTMUCH_DATABASE_PATH NULL

// DISPLAY
#define MAX_THREADS 100
#define MAX_MSGS_PER_THREAD 100

// QUERIES
#define QUERY_LIST                                                      \
	Q(INBOX,  "INBOX",  "tag:inbox")                                    \
	Q(ARCHIV, "ARCHIV", "tag:archive")                                  \
	Q(ATTCH,  "ATTCH",  "tag:attachment")                               \
	Q(SENT,   "SENT",   "from:@iamjuo.com")                             \
	Q(IBKR,   "IBKR",   "from:@interactivebrokers.com")                 \
	Q(SUCKLS, "SUCKLS", "to:@suckless.org")                             \
	Q(ALPINE, "ALPINE", "to:@alpinelinux.org")                          \
	Q(NTMUCH, "NTMUCH", "to:@notmuchmail.org")                          \
	Q(SMTPD,  "SMTPD",  "to:@opensmtpd.org")                            \
	Q(TRASH,  "TRASH",  "tag:delete")                                   \
	Q(JUNK,   "JUNK",   "folder:Junk")                                  \
	Q(ALL,    "ALL",    "*")

#define Q(name, label, query) Q_##name,
enum { QUERY_LIST Q_COUNT };
#undef Q

// KEYS
#define KEY_QUIT    'q'
#define KEY_UP      'w'
#define KEY_DOWN    's'
#define KEY_BACK    'a'
#define KEY_ACTION  'd'
#define KEY_TOP     'g'
#define KEY_BOT     'G'
#define KEY_Q_NEXT  'n'
#define KEY_Q_PREV  'p'
#define KEY_COMPOSE 'm'
#define KEY_REPLY   'r'
#define KEY_UNFOLD  TB_KEY_SPACE

// COLOR MODE
#define COL_MODE TB_OUTPUT_NORMAL

// DECORATIONS, read termbox2.h for ideas
#define DEC_DATE   TB_MAGENTA,               TB_DEFAULT
#define DEC_MSGS   TB_MAGENTA,               TB_DEFAULT
#define DEC_FROM   TB_YELLOW,                TB_DEFAULT
#define DEC_TREE   TB_MAGENTA,               TB_DEFAULT
#define DEC_SUBJ   TB_BLUE,                  TB_DEFAULT
#define DEC_TAGS   TB_MAGENTA,               TB_DEFAULT
#define DEC_STAT   ( TB_DEFAULT | TB_BOLD ), TB_DEFAULT
#define DEC_SEL_FG TB_UNDERLINE
#define DEC_SEL_BG TB_DEFAULT

// FORMAT
#define STRFTIME_BASE  "%b %d"
#define STRFTIME_TODAY "%H:%M"

#define FMT_STAT " NeoMuch: %s %d/%d # %s rows:%d sel:%d", qlabel[qid], msg_unread, msg_count, qstr[qid], rows, sel
#define FMT_DATE " %-6s"   , date
#define FMT_MSGS " [%d/%d]", j + 1, t->msgs
#define FMT_FROM " %.18s"  , from
#define FMT_TREE "%s"      , tree
#define FMT_SUBJ " %s"     , subj
#define FMT_TAGS " # %s"   , m->tags
#define FMT_DATE_WIDTH 1 + 6
#define FMT_MSGS_WIDTH 4 +      DIGITS(t->msgs) + DIGITS(j + 1)
#define FMT_FROM_WIDTH 1 + 18 - DIGITS(t->msgs) - DIGITS(j + 1)
#define FMT_TREE_WIDTH 0 + utf8_width(tree)
#define FMT_SUBJ_WIDTH 1 + utf8_width(subj)

#define DIGITS(n) ((n) >= 100 ? 3 : (n) >= 10 ? 2 : 1)
