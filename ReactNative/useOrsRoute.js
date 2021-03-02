import openRouteService from '../api/openRouteService';

export default () => {
    /**
     *
     * @param {Object[]} [locations={}]
     * @param {number[]} [locations[].longitude]
     * @param {number[]} [locations[].latitude]
     * @param {boolean} geometry
     * @returns {object} {length, time, geometry[]}
     */
    const getLengthTimeGeo = async (locations, geometry) => {
        try {
            const response = await openRouteService.post('/v2/directions/driving-car', {
                coordinates: fillCoordinates(locations),
                instructions: false,
                geometry: geometry,
            });
            return {
                length: response.data.routes[0].summary.distance,
                time: response.data.routes[0].summary.duration,
                geometry: geometry ? decodePolyline(response.data.routes[0].geometry) : null,
            };
        } catch (error) {
            console.log(error);
            return null;
        }
    };

    function fillCoordinates(locations) {
        let coordinates = [];
        for (let i = 0; i < locations.length; i++) {
            coordinates.push([locations[i].longitude, locations[i].latitude]);
        }
        return coordinates;
    }

    const decodePolyline = (encodedPolyline, includeElevation) => {
        // array that holds the points
        let points = [];
        let index = 0;
        const len = encodedPolyline.length;
        let lat = 0;
        let lng = 0;
        let ele = 0;
        while (index < len) {
            let b;
            let shift = 0;
            let result = 0;
            do {
                b = encodedPolyline.charAt(index++).charCodeAt(0) - 63; // finds ascii
                // and subtract it by 63
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);

            lat += (result & 1) !== 0 ? ~(result >> 1) : result >> 1;
            shift = 0;
            result = 0;
            do {
                b = encodedPolyline.charAt(index++).charCodeAt(0) - 63;
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);
            lng += (result & 1) !== 0 ? ~(result >> 1) : result >> 1;

            if (includeElevation) {
                shift = 0;
                result = 0;
                do {
                    b = encodedPolyline.charAt(index++).charCodeAt(0) - 63;
                    result |= (b & 0x1f) << shift;
                    shift += 5;
                } while (b >= 0x20);
                ele += (result & 1) !== 0 ? ~(result >> 1) : result >> 1;
            }
            try {
                let location = { latitude: lat / 1e5, longitude: lng / 1e5 };
                if (includeElevation) location.push(ele / 100);
                points.push(location);
            } catch (e) {
                console.log(e);
            }
        }
        return points;
    };

    return [getLengthTimeGeo];
};
