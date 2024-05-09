#pragma once
#include "model.h"
#include "sqlite3.h"

#include <QDebug>

namespace dh {

class Database {
private:
    sqlite3 *_db = nullptr;

    static sqlite3 *openOrCreateDB()
    {
        sqlite3 *db = nullptr;
        auto rc = sqlite3_open("dog_hunter.db", &db);

        if (rc) {
            qDebug() <<  "Can't open database: " << sqlite3_errmsg(db);
            return nullptr;
        } else {
            qDebug() << "Opened database successfully";
        }

        // check db on empty
        int count;

        rc = sqlite3_exec(db, "SELECT COUNT(*) FROM DogOwners;", [](void* data, int argc, char** argv, char**) {
                if (argc > 0)
                    *reinterpret_cast<int*>(data) = std::stoi(argv[0]);
                return 0;
            }, &count, nullptr);

        if (rc != SQLITE_OK) {
            qDebug() << "SQL error: " << sqlite3_errmsg(db);
            sqlite3_close(db);
            return nullptr;
        }

        if(count > 0)
        {
            // database has already been created
            return db;
        }

        // tables
        auto execTable = [&](const auto *tableExec) {
            rc = sqlite3_exec(db, tableExec, nullptr, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                qDebug() << "SQL error: " << sqlite3_errmsg(db);
                sqlite3_close(db);
                return false;
            }

            return true;
        };

        if(!execTable("CREATE TABLE IF NOT EXISTS DogOwners ("
                         "id STRING PRIMARY KEY,"
                         "name TEXT,"
                         "password TEXT,"
                         "age INTEGER"
                         ");"))
            return nullptr;

        if(!execTable("CREATE TABLE IF NOT EXISTS Dogs ("
                         "id INTEGER PRIMARY KEY,"
                         "name TEXT,"
                         "age INTEGER"
                         ");"))
            return nullptr;

        if(!execTable("CREATE TABLE IF NOT EXISTS DogsByOwners ("
                         "id INTEGER PRIMARY KEY,"
                         "dog_owner STRING NOT NULL,"
                         "dog_id INTEGER,"
                         "FOREIGN KEY (dog_owner) REFERENCES DogOwners(dog_owner),"
                         "FOREIGN KEY (dog_id) REFERENCES Dogs(dog)"
                         ");"))
            return nullptr;

        // ... another tables

        // filling dummy data
        if(!execTable("INSERT INTO DogOwners (id, name, password, age) VALUES "
                                         "('1@gmail.com', 'John', '123', 30),"
                                         "('2@gmail.com', 'Emily', '123', 25),"
                                         "('3@gmail.com', 'Michael', '123', 40),"
                                         "('4@gmail.com', 'Sophia', '123', 35),"
                                         "('5@gmail.com', 'David', 'password5', 28);"))
            return nullptr;

        // Insert sample data into Dogs table
        if(!execTable("INSERT INTO Dogs (id, name, age) VALUES "
                                    "(1, 'Buddy', 3),"
                                    "(2, 'Max', 5),"
                                    "(3, 'Charlie', 2),"
                                    "(4, 'Bella', 4),"
                                    "(5, 'Lucy', 6);"))
            return nullptr;

        // Insert sample data into DogsByOwners table
        if(!execTable("INSERT INTO DogsByOwners (dog_owner, dog_id) VALUES "
                                            "('1@gmail.com', 1),"
                                            "('2@gmail.com', 2),"
                                            "('3@gmail.com', 3),"
                                            "('4@gmail.com', 4),"
                                            "('5@gmail.com', 5);"))
            return nullptr;

        qDebug() << "Database was successfully created";
        return db;
    }

public:
    // Static member function to access the singleton instance
    static Database &getInstance() {
        static Database instance(openOrCreateDB());
        return instance;
    }
        // Delete copy constructor and assignment operator
    Database(const Database&) = delete;
    void operator=(const Database&) = delete;

    ~Database() {
        if(_db)
            sqlite3_close(_db);
    }

    enum class QueryResult : int
    {
        Ok = 0,
        WrongLogin,
        WrongPassword,
        WrongDatabase,
    };

    DogOwnerPtr getDogOwner(const std::string &login, const std::string &password, QueryResult &res) const
    {
        if(!_db)
        {
            qDebug() << "Database wasn't initialised!";
            res = QueryResult::WrongDatabase;
            return nullptr;
        }

        const auto *sql = "SELECT * FROM DogOwners WHERE id = ?";
        sqlite3_stmt* stmt = nullptr;

        auto rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            qDebug() << "SQL error: " << sqlite3_errmsg(_db);
            res = QueryResult::WrongDatabase;
            return nullptr;
        }

        // Bind the parameters
        sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

        // Execute the query
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            res = QueryResult::WrongLogin;
            return nullptr;
        }

        auto dogOwner = std::make_shared<DogOwner>();
        dogOwner->_email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        dogOwner->_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        dogOwner->_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if(dogOwner->_password != password)
        {
            res = QueryResult::WrongPassword;
            return nullptr;
        }

        sql = "SELECT * FROM DogsByOwners JOIN Dogs ON DogsByOwners.dog_id = Dogs.id WHERE dog_owner = ?";
        sqlite3_stmt* stmt_dogs;

        rc = sqlite3_prepare_v2(_db, sql, -1, &stmt_dogs, nullptr);
        if (rc != SQLITE_OK) {
            qDebug()  << "SQL error: " << sqlite3_errmsg(_db);
            sqlite3_finalize(stmt);
            res = QueryResult::WrongDatabase;
            return nullptr;
        }

        sqlite3_bind_text(stmt_dogs, 1, login.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt_dogs) == SQLITE_ROW) {
            auto dog = std::make_shared<Dog>(reinterpret_cast<const char*>(sqlite3_column_text(stmt_dogs, 5)), *dogOwner);
            dog->age = sqlite3_column_int(stmt_dogs, 6);
            dogOwner->_dogs.emplace(dog->_name, dog);
        }

        sqlite3_finalize(stmt_dogs);
        sqlite3_finalize(stmt);

        res = QueryResult::Ok;
        return dogOwner;
    }

private:
    // Private constructor to prevent external instantiation
    Database(sqlite3 *db) : _db(db) {}
};

}
