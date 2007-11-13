/*
  Copyright (C) 2007 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#ifndef SQLDB_DATABASECONNECTION_H
#define SQLDB_DATABASECONNECTION_H

#include "ConnectionParameters.h"
#include <kexidb/connection.h>
#include <kexidb/driver.h>
#include <ksharedptr.h>

namespace SQLDB
{
    /** Wrapper for connection data and connection.
     *
     * Needed because connection data life time is bound to
     * connection, but this is not enforced in Kexi.
     */
    class KexiConnection: public KShared
    {
    public:
        KexiConnection(KexiDB::Driver& driver,
                       const ConnectionParameters& connParams,
                       const QString& fileName=QString::null);

        KexiDB::Connection& kexi() const
        {
            return *_conn;
        }

    private:
        KexiDB::ConnectionData _cd;
        KexiDB::Connection* _conn;
    };

    typedef KSharedPtr<KexiConnection> DatabaseConnection;
}

#endif /* SQLDB_DATABASECONNECTION_H */
