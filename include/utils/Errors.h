#pragma once

enum class Errors
{
    NoError,
    CatchError,
    DatabaseConnectionFailed,
    TransactionStartFailed,
    StatementInitFailed,
    StatementPrepareFailed,
    BindParamFailed,
    ExecutionFailed,
    NoRowsAffected,
    StoreResultFailed,
    FetchFailed,
    CommitFailed,
    UnknownError,
    BindResultFailed
};