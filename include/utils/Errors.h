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
    NoRowsFound,
    StoreResultFailed,
    FetchFailed,
    CommitFailed,
    UnknownError,
    BindResultFailed
};