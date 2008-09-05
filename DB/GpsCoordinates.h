/* Copyright (C) 2008 Tuomas Suutari <thsuut@utu.fi>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DB_GPSCOORDINATES_H
#define DB_GPSCOORDINATES_H

#include "config-kpa-marble.h"
#ifdef HAVE_MARBLE
#   include <marble/GeoDataCoordinates.h>
#endif

namespace DB {

/** Stores a position on the globe with optional precision.
 *
 * Longitude and latitude are stored as degrees and altitude and
 * precision as meters.
 */
class GpsCoordinates
{
public:
    /** Value for precision field when there is no precision data
     * available.
     */
    static const int NoPrecisionData = -1;

    /** Value for precision field for null instance.
     */
    static const int PrecisionDataForNull = -2;


    GpsCoordinates()
        throw()
        : _longitude(0.0)
        , _latitude(0.0)
        , _altitude(0)
        , _precision(PrecisionDataForNull)
    {
    }

    GpsCoordinates(
        double longitude,
        double latitude,
        double altitude,
        int precision=NoPrecisionData)
        throw()
        : _longitude(longitude)
        , _latitude(latitude)
        , _altitude(altitude)
        , _precision(precision)
    {
        Q_ASSERT(_precision >= 0 || _precision == NoPrecisionData);
        Q_ASSERT(!this->isNull());
    }

#ifdef HAVE_MARBLE
    GpsCoordinates(const GeoDataCoordinates& position)
        throw()
        : _longitude(0.0)
        , _latitude(0.0)
        , _altitude(position.altitude())
        , _precision(NoPrecisionData)
    {
        // Get the coordinates from the given position to our member
        // variables
        position.geoCoordinates(_longitude, _latitude, GeoDataCoordinates::Degree);

        Q_ASSERT(!this->isNull());
    }
#endif

    // implicit copy constructor
    // implicit assign operator

    bool operator==(const GpsCoordinates& other) const throw()
    {
        if (this->isNull())
            return other.isNull();
        else
            return
                this->longitude() == other.longitude() &&
                this->latitude() == other.latitude() &&
                this->altitude() == other.altitude() &&
                this->precision() == other.precision();
    }

    bool operator!=(const GpsCoordinates& other) const throw()
    {
        return !(*this == other);
    }

    bool isNull() const throw()
    {
        return _precision == PrecisionDataForNull;
    }

    double longitude() const throw()
    {
        return _longitude;
    }

    double latitude() const throw()
    {
        return _latitude;
    }

    double altitude() const throw()
    {
        return _altitude;
    }

    double precision() const throw()
    {
        return _precision;
    }

#ifdef HAVE_MARBLE
    GeoDataCoordinates toGeoDataCoordinates() const throw()
    {
        return GeoDataCoordinates(
            _longitude,
            _latitude,
            _altitude,
            GeoDataCoordinates::Degree);
    }
#endif

private:
    /** Longitude in degrees.
     */
    double _longitude;

    /** Latitude in degrees.
     */
    double _latitude;

    /** Altitude in meters.
     */
    double _altitude;

    /** Precision in meters.
     */
    int _precision;
};

}

#endif /* DB_GPSCOORDINATES_H */