#include <string.h>

#include <mstd/pointer_cast.hpp>
#include <mstd/performance_timer.hpp>
#include <mstd/strings.hpp>
#include <mstd/utf8.hpp>

#include <mlog/Logging.h>

#if defined(_MSC_VER)
#include "internal/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "SQLite.h"

MLOG_DECLARE_LOGGER(sqlite);

using namespace std;

namespace sqlite {

//////////////////////////////////////////////////////////////////////////
// class SafeHolder
//////////////////////////////////////////////////////////////////////////

template<class Object, int (*Finalizer)(Object * object)>
class SafeHolder {
public:
    SafeHolder()
        : object_(NULL)
    {
    }
    
    ~SafeHolder()
    {
        if(object_)
            Finalizer(object_);
    }
    
    Object * operator*()
    {
        return handle();
    }
    
    Object * handle()
    {
        return object_;
    }
    
    Object ** writeHandle()
    {
        return & object_;
    }
private:
    Object * object_;
};

//////////////////////////////////////////////////////////////////////////
// class SQLite::Impl
//////////////////////////////////////////////////////////////////////////

class SQLite::Impl {
public:
    Impl(const std::string & filename)
    {
        check(sqlite3_open(filename.c_str(), inst.writeHandle()));
    }

    sqlite3 * handle()
    {
        return inst.handle();
    }

    void check(int err)
    {
        if(err != SQLITE_OK)
        {
            const char * msg = sqlite3_errmsg(*inst);
            throw SQLiteException() << mstd::error_message(msg) << mstd::error_no(err);
        }
    }
private:
    SafeHolder<sqlite3, &sqlite3_close> inst;
};

//////////////////////////////////////////////////////////////////////////
// class SQLite
//////////////////////////////////////////////////////////////////////////

SQLite::SQLite(const std::string & filename)
    : impl_(new Impl(filename)) {}

SQLite::SQLite(const std::wstring & filename)
    : impl_(new Impl(mstd::utf8(filename))) {}
    
void SQLite::exec(const std::string & sql)
{
    SQLiteStatement stmt(*this, sql);
    stmt.step();
}

SQLite::~SQLite()
{
}

//////////////////////////////////////////////////////////////////////////
// class SQLiteStatement::Impl
//////////////////////////////////////////////////////////////////////////

class SQLiteStatement::Impl {
public:
    Impl(SQLite & db, const std::string & sql)
        : db_(*db.impl_), sql_(sql)
    {
        const char * dummy;

        db_.check(sqlite3_prepare_v2(db_.handle(), sql.c_str(), sql.length(), inst.writeHandle(), &dummy));
    }

    void bindInt(int index, boost::int32_t value)
    {
        sqlite3_bind_int(*inst, index, value);
    }

    void bindInt64(int index, boost::int64_t value)
    {
        sqlite3_bind_int64(*inst, index, value);
    }

    void bindString(int index, const std::string & value)
    {
        sqlite3_bind_text(*inst, index, value.c_str(), value.length(), SQLITE_STATIC);
    }

    void reset()
    {
        db_.check(sqlite3_reset(*inst));
    }
    
    bool step()
    {
#ifndef __APPLE__
        mstd::performance_timer ptimer;
#endif
        int err = sqlite3_step(*inst);
#ifndef __APPLE__
        uint64_t cur = ptimer.microseconds();
        if(cur > 100000)
            MLOG_MESSAGE(Warning, "Slow query: " << cur << ", sql: " << sql_);
#endif
        if(err == SQLITE_ROW)
            return true;
        else if(err == SQLITE_DONE)
            return false;
        else {
            db_.check(err);
            return false;
        }
    }

    int getInt(int col)
    {
        return sqlite3_column_int(*inst, col);
    }

    boost::int64_t getInt64(int col)
    {
        return sqlite3_column_int64(*inst, col);
    }

    double getDouble(int col)
    {
        return sqlite3_column_int(*inst, col);
    }

    std::string getString(int col)
    {
        return mstd::pointer_cast<const char*>(sqlite3_column_text(*inst, col));
    }

    std::wstring getWString(int col)
    {
        const char * res = mstd::pointer_cast<const char*>(sqlite3_column_text(*inst, col));
        size_t len = strlen(res);
        wchar_t * buf = mstd::pointer_cast<wchar_t*>(alloca(sizeof(wchar_t) * (len + 1)));
        wchar_t * end = mstd::deutf8(res, res + len, buf);
        return std::wstring(buf, end);
    }
private:
    SQLite::Impl & db_;
    SafeHolder<sqlite3_stmt, sqlite3_finalize> inst;
    std::string sql_;
};

//////////////////////////////////////////////////////////////////////////
// class SQLiteStatement
//////////////////////////////////////////////////////////////////////////

SQLiteStatement::SQLiteStatement(SQLite & db, const std::string & sql)
    : impl_(new Impl(db, sql)) {}

SQLiteStatement::SQLiteStatement(SQLite & db, const std::wstring & sql)
    : impl_(new Impl(db, mstd::utf8(sql))) {}

SQLiteStatement::~SQLiteStatement()
{
}

void SQLiteStatement::bindInt(int index, boost::int32_t value)
{
    impl_->bindInt(index, value);
}

void SQLiteStatement::bindInt64(int index, boost::int64_t value)
{
    impl_->bindInt64(index, value);
}

void SQLiteStatement::bindString(int index, const std::string & value)
{
    impl_->bindString(index, value);
}

void SQLiteStatement::reset()
{
    impl_->reset();
}

bool SQLiteStatement::step()
{
    return impl_->step();
}

int SQLiteStatement::getInt(int col)
{
    return impl_->getInt(col);
}

boost::int64_t SQLiteStatement::getInt64(int col)
{
    return impl_->getInt64(col);
}

double SQLiteStatement::getDouble(int col)
{
    return impl_->getDouble(col);
}

std::string SQLiteStatement::getString(int col)
{
    return impl_->getString(col);
}

std::wstring SQLiteStatement::getWString(int col)
{
    return impl_->getWString(col);
}

Transaction::Transaction(SQLite & db)
    : db_(db), ok_(false)
{
    db_.exec("begin");
}

Transaction::~Transaction()
{
    if(!ok_)
        db_.exec("rollback");
}

void Transaction::commit()
{
    db_.exec("commit");
    ok_ = true;
}

}
