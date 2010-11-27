#pragma once

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/cstdint.hpp>
#include <mstd/exception.hpp>

namespace sqlite {

class SQLite;

class SQLiteStatement : private boost ::noncopyable {
public:
    SQLiteStatement(SQLite & db, const std::string & sql);
    SQLiteStatement(SQLite & db, const std::wstring & sql);
    ~SQLiteStatement();

    void reset();
    bool step();

    void bindInt64(int index, boost::int64_t value);
    void bindInt(int index, boost::int32_t value);
    void bindString(int index, const std::string & value);
    
    template<class T, class S>
    typename boost::enable_if<boost::is_same<T, boost::int64_t>, void>::type
    bind(int index, S value)
    {
        bindInt64(index, value);
    }

    int getInt(int col);
    boost::int64_t getInt64(int col);
    double getDouble(int col);
    std::string getString(int col);
    std::wstring getWString(int col);
private:
    class Impl;
    
    friend class SQLite;

    boost::scoped_ptr<Impl> impl_;
};

class SQLite : private boost::noncopyable {
public:
    SQLite(const std::string & filename);
    SQLite(const std::wstring & filename);

    void exec(const std::string & sql);

    ~SQLite();
private:
    class Impl;
    
    friend class SQLiteStatement::Impl;
    
    boost::scoped_ptr<Impl> impl_;
};

class Transaction : public boost::noncopyable {
public:
    explicit Transaction(SQLite & db);
    ~Transaction();

    void commit();
private:
    SQLite & db_;
    bool ok_;
};

typedef mstd::own_exception<SQLite> SQLiteException;

}
