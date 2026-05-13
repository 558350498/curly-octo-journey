#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOK_KEYWORD = 1,
    TOK_IDENTIFIER = 2,
    TOK_UINT = 3,
    TOK_OPERATOR = 4,
    TOK_DELIMITER = 5
} TokenType;

typedef struct {
    char **items;
    size_t size;
    size_t cap;
} SymbolTable;

static const char *KEYWORDS[] = {
    "if", "int", "for", "while", "do", "return", "break", "continue", "else"
};
static const char *DELIMITERS[] = {",", ";", "{", "}", "(", ")"};
static const char *OPERATORS[] = {
    "+", "-", "*", "/", "=", ">", "<", ">=", "<=", "!=", ":="
};

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void st_init(SymbolTable *st) {
    st->cap = 16;
    st->size = 0;
    st->items = (char **)calloc(st->cap, sizeof(char *));
    if (!st->items) die("oom");
}

static void st_free(SymbolTable *st) {
    for (size_t i = 0; i < st->size; i++) free(st->items[i]);
    free(st->items);
    st->items = NULL;
    st->size = st->cap = 0;
}

static int st_find_or_add(SymbolTable *st, const char *lexeme) {
    for (size_t i = 0; i < st->size; i++) {
        if (strcmp(st->items[i], lexeme) == 0) return (int)i + 1;
    }
    if (st->size == st->cap) {
        st->cap *= 2;
        char **n = (char **)realloc(st->items, st->cap * sizeof(char *));
        if (!n) die("oom");
        st->items = n;
    }
    st->items[st->size] = strdup(lexeme);
    if (!st->items[st->size]) die("oom");
    st->size++;
    return (int)st->size;
}

static int lookup(const char *lexeme, const char *const *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (strcmp(lexeme, arr[i]) == 0) return (int)i + 1;
    }
    return 0;
}

static void emit(const char *lexeme, TokenType t, int index) {
    printf("(%d,\"%s\",%d)\n", (int)t, lexeme, index);
}

int main(int argc, char **argv) {
    const char *path = (argc >= 2) ? argv[1] : "example.txt";
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return 1;
    }

    SymbolTable id_table, const_table;
    st_init(&id_table);
    st_init(&const_table);

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (isspace(ch)) continue;

        if (isalpha(ch) || ch == '_') {
            char buf[256];
            size_t len = 0;
            do {
                if (len + 1 < sizeof(buf)) buf[len++] = (char)ch;
                ch = fgetc(fp);
            } while (ch != EOF && (isalnum(ch) || ch == '_'));
            if (ch != EOF) ungetc(ch, fp);
            buf[len] = '\0';

            int k = lookup(buf, KEYWORDS, sizeof(KEYWORDS) / sizeof(KEYWORDS[0]));
            if (k) emit(buf, TOK_KEYWORD, k - 1);
            else emit(buf, TOK_IDENTIFIER, st_find_or_add(&id_table, buf) - 1);
            continue;
        }

        if (isdigit(ch)) {
            char buf[256];
            size_t len = 0;
            do {
                if (len + 1 < sizeof(buf)) buf[len++] = (char)ch;
                ch = fgetc(fp);
            } while (ch != EOF && isdigit(ch));
            if (ch != EOF) ungetc(ch, fp);
            buf[len] = '\0';
            emit(buf, TOK_UINT, st_find_or_add(&const_table, buf) - 1);
            continue;
        }

        char two[3] = {(char)ch, '\0', '\0'};
        int next = fgetc(fp);
        if (next != EOF) {
            two[1] = (char)next;
            int op2 = lookup(two, OPERATORS, sizeof(OPERATORS) / sizeof(OPERATORS[0]));
            if (op2) {
                emit(two, TOK_OPERATOR, op2 - 1);
                continue;
            }
            ungetc(next, fp);
            two[1] = '\0';
        }

        int op1 = lookup(two, OPERATORS, sizeof(OPERATORS) / sizeof(OPERATORS[0]));
        if (op1) {
            emit(two, TOK_OPERATOR, op1 - 1);
            continue;
        }
        int d = lookup(two, DELIMITERS, sizeof(DELIMITERS) / sizeof(DELIMITERS[0]));
        if (d) {
            emit(two, TOK_DELIMITER, d - 1);
            continue;
        }

        fprintf(stderr, "Error: unknown char '%c'\n", (char)ch);
    }

    st_free(&id_table);
    st_free(&const_table);
    fclose(fp);
    return 0;
}
