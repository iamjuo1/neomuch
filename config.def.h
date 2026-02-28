// QUERIES
#define QUERY_LIST                        \
	Q(INBOX,  "INBOX",  "tag:inbox")      \
	Q(ARCHIV, "ARCHIV", "tag:archive")    \
	Q(ATTCH,  "ATTCH",  "tag:attachment") \
	Q(SENT,   "SENT",   "tag:SENT")       \
	Q(IBKR,   "IBKR",   "tag:IBKR")       \
	Q(T212,   "T212",   "tag:T212")       \
	Q(TASTY,  "TASTY",  "tag:TASTY")      \
	Q(WISE,   "WISE",   "tag:WISE")       \
	Q(FWD,    "FWD",    "tag:FWD")        \
	Q(SUCKLS, "SUCKLS", "tag:SUCKLS")     \
	Q(ALPINE, "ALPINE", "tag:ALPINE")     \
	Q(NTMUCH, "NTMUCH", "tag:NTMUCH")     \
	Q(SMTPD,  "SMTPD",  "tag:SMTPD")      \
	Q(TRASH,  "TRASH",  "tag:TRASH")      \
	Q(JUNK,   "JUNK",   "folder:Junk")    \
	Q(ALL,    "ALL",    "*")

#define Q(name, label, query) Q_##name,
enum { QUERY_LIST Q_COUNT };
#undef Q

enum
{
	KEY_QUIT    = 'q',
	KEY_UP      = 'w',
	KEY_DOWN    = 's',
	KEY_BACK    = 'a',
	KEY_ACTION  = 'd',
	KEY_TOP     = 'g',
	KEY_BOT     = 'G',
	KEY_Q_NEXT  = 'n',
	KEY_Q_PREV  = 'p',
	KEY_COMPOSE = 'm',
	KEY_REPLY   = 'r',
	KEY_UNFOLD  = TB_KEY_SPACE,
	KEY_SIDEBAR = TB_KEY_TAB,

	// DISPLAY
	MAX_THREADS = 200,
	MAX_MSGS_PER_THREAD = 50,

	// COLOR MODE
	COL_MODE = TB_OUTPUT_NORMAL,
};

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
#define FMT_DATE " %-6s"               , date
#define FMT_MSGS " [%d/%d]"            , j + 1, t->msgs
#define FMT_FROM " %.18s"              , from
#define FMT_TREE (tree ? " %s" : "%s") , tree
#define FMT_SUBJ (subj ? " %s" : "%s") , subj
#define FMT_TAGS " # %s"               , m->tags
// #define FMT_TAGS "%20s %20s" , m->parent_id , m->id
#define FMT_DATE_WIDTH 1 + 6
#define FMT_MSGS_WIDTH 1 + 3  + DIGITS(t->msgs) + DIGITS(j + 1)
#define FMT_FROM_WIDTH 1 + 18 - DIGITS(t->msgs) - DIGITS(j + 1)
#define FMT_TREE_WIDTH (tree ? 1 : 0) + utf8_width(tree)
#define FMT_SUBJ_WIDTH (subj ? 1 : 0) + utf8_width(subj)

#define DIGITS(n) ((n) >= 100 ? 3 : (n) >= 10 ? 2 : 1)
