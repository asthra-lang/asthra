typedef enum {
    // Declarations
    AST_PACKAGE_DECL,
    AST_IMPORT_DECL,
    AST_FUNCTION_DECL,
    AST_STRUCT_DECL,
    AST_IMPL_BLOCK,
    AST_METHOD_DECL,
    AST_EXTERN_DECL,
    AST_CONST_DECL, // const NAME: Type = value;

    // Statements
    AST_VAR_DECL,
    AST_ASSIGN_STMT,
    AST_IF_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_EXPR_STMT,
    AST_SPAWN_STMT,
    AST_SPAWN_WITH_HANDLE_STMT,
    AST_MATCH_STMT,
    AST_UNSAFE_BLOCK,
    // Note: AST_WORKER_POOL_DECL, AST_CLOSE_STMT moved to stdlib

    // Expressions
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_CALL_EXPR,
    AST_FIELD_ACCESS,
    AST_ARRAY_ACCESS,
    AST_STRUCT_LITERAL,
    AST_ARRAY_LITERAL,
    AST_ENUM_VARIANT,
    AST_TASK_HANDLE,
    // Note: AST_TIMEOUT_EXPR moved to stdlib

    // Literals
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_BOOL_LITERAL,
    AST_UNIT_LITERAL,
    AST_IDENTIFIER,

    // Const expressions
    AST_CONST_EXPR, // Compile-time constant expression

    // Types
    AST_TYPE,
    AST_BASE_TYPE,
    AST_SLICE_TYPE,
    AST_PTR_TYPE,
    AST_POINTER_TYPE,
    AST_RESULT_TYPE,
    AST_OPTION_TYPE,
    // Note: Tier 3 types moved to stdlib:
    // AST_CHANNEL_TYPE, AST_TASK_HANDLE_TYPE, AST_WORKER_POOL_TYPE

    // Patterns
    AST_ENUM_PATTERN,
    AST_STRUCT_PATTERN,
    AST_FIELD_PATTERN,

    // Special
    AST_BLOCK,
    AST_PARAMETER,
    AST_ARGUMENT_LIST,
    AST_FIELD_LIST,
    AST_MATCH_ARM,
    AST_PROGRAM,

    AST_NODE_TYPE_COUNT
} ASTNodeType;

// Advanced concurrency structures
typedef struct {
    char *function_name;
    ASTNodeList *args;
    char *handle_var_name; // Variable to store the task handle
} SpawnWithHandleStmt;

typedef struct {
    ASTNode *task_handle_expr; // Expression that evaluates to a task handle
    ASTNode *timeout_expr;     // Optional timeout expression
} AwaitExpr;

/*
 * Note: Tier 3 concurrency structures moved to stdlib
 *
typedef struct {
    char *channel_name;
    ASTNode *element_type;      // Type of elements in the channel
    ASTNode *buffer_size;       // Optional buffer size (NULL for unbuffered)
} ChannelDecl;

typedef struct {
    ASTNode *channel_expr;      // Channel to send to
    ASTNode *value_expr;        // Value to send
} SendStmt;

typedef struct {
    ASTNode *channel_expr;      // Channel to receive from
    ASTNode *timeout_expr;      // Optional timeout expression
} RecvExpr;

typedef struct {
    ASTNodeList *cases;         // List of select cases
    ASTNode *default_case;      // Optional default case
} SelectStmt;

typedef struct {
    enum {
        SELECT_CASE_RECV,
        SELECT_CASE_SEND,
        SELECT_CASE_DEFAULT
    } case_type;
    ASTNode *channel_expr;      // Channel for recv/send
    ASTNode *value_expr;        // Value for send case
    char *binding_var;          // Variable binding for recv case
    ASTNode *body;              // Case body
} SelectCase;

typedef struct {
    char *pool_name;
    ASTNode *worker_count;      // Number of workers
    ASTNode *queue_size;        // Task queue size
} WorkerPoolDecl;

typedef struct {
    ASTNode *channel_expr;      // Channel to close
} CloseStmt;

typedef struct {
    ASTNode *element_type;      // Type of channel elements
    ASTNode *buffer_size;       // Optional buffer size
} ChannelType;

typedef struct {
    ASTNode *result_type;       // Type of task result
} TaskHandleType;

typedef struct {
    ASTNode *task_type;         // Type of tasks in pool
} WorkerPoolType;

typedef struct {
    ASTNode *expr;              // Expression to timeout
    ASTNode *duration;          // Timeout duration
} TimeoutExpr;
*/

// Update the union in ASTNode
typedef union {
    // ... existing fields ...
    SpawnStmt spawn_stmt;
    SpawnWithHandleStmt spawn_with_handle_stmt;
    AwaitExpr await_expr;
    // Note: Tier 3 union fields moved to stdlib:
    // ChannelDecl channel_decl;
    // SendStmt send_stmt;
    // RecvExpr recv_expr;
    // SelectStmt select_stmt;
    // SelectCase select_case;
    // ChannelType channel_type;
    // TaskHandleType task_handle_type;
    // WorkerPoolDecl worker_pool_decl;
    // CloseStmt close_stmt;
    // WorkerPoolType worker_pool_type;
    // TimeoutExpr timeout_expr;
    // ... existing fields ...
} ASTNodeData;
