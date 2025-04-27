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
    CommitFailed,
    UnknownError
};