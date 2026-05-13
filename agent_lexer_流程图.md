# Agent Lexer 词法分析流程图

> 对应实现文件：`agent_lexer.c`

```mermaid
flowchart TD
    A([Start]) --> B[读取命令行参数\n默认输入 example.txt]
    B --> C{fopen 成功?}
    C -- 否 --> C1[perror 输出错误] --> C2([Exit 1])
    C -- 是 --> D[初始化符号表\nid_table / const_table]

    D --> E[循环读取 ch = fgetc(fp)]
    E --> F{ch == EOF?}
    F -- 是 --> Z[释放符号表并 fclose] --> Z1([Exit 0])
    F -- 否 --> G{isspace(ch)?}

    G -- 是 --> E
    G -- 否 --> H{isalpha(ch) 或 '_' ?}

    H -- 是 --> H1[读取标识符串\n[a-zA-Z_][a-zA-Z0-9_]*]
    H1 --> H2{是关键字?}
    H2 -- 是 --> H3[emit KEYWORD(1)] --> E
    H2 -- 否 --> H4[写入/查询 id_table\nemit IDENTIFIER(2)] --> E

    H -- 否 --> I{isdigit(ch)?}
    I -- 是 --> I1[读取整数字串\n[0-9]+]
    I1 --> I2[写入/查询 const_table\nemit UINT(3)] --> E

    I -- 否 --> J[尝试双字符运算符\n读取 next 组成 two-char]
    J --> K{two-char 在 OPERATORS?}
    K -- 是 --> K1[emit OPERATOR(4)\n如 >= <= != :=] --> E
    K -- 否 --> L[ungetc(next)\n回退为单字符]

    L --> M{单字符在 OPERATORS?}
    M -- 是 --> M1[emit OPERATOR(4)] --> E
    M -- 否 --> N{单字符在 DELIMITERS?}
    N -- 是 --> N1[emit DELIMITER(5)] --> E
    N -- 否 --> O[stderr: unknown char error] --> E
```
