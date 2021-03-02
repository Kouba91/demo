import React, { useState, useEffect } from 'react';
import { View, Text, StyleSheet, Modal, TouchableOpacity, Dimensions, TextInput, Keyboard } from 'react-native';
import MapView, { Marker } from 'react-native-maps';
import { Entypo } from '@expo/vector-icons';
import { AntDesign } from '@expo/vector-icons';
import { FontAwesome5 } from '@expo/vector-icons';
import useOrsReverseGeo from '../hooks/useOrsReverseGeo';
import useSuggest from '../hooks/useSuggest';

const ModalMapSuggestComponent = (props) => {
  const [point, setPoint] = useState(null);
  const [showModal, setShowModal] = useState(false);

  const [getNameFromCoords] = useOrsReverseGeo(null);
  const [labelMap, setLabelMap] = useState(null);
  const [labelSuggest, setLabelSuggest] = useState(null);

  const [suggestApi, response] = useSuggest();
  const [query, setQuery] = useState(null);

  useEffect(() => {
    const timeOutId = setTimeout(async () => {
      suggestApi(query, 'address|street|poi');
    }, 250);
    return () => clearTimeout(timeOutId);
  }, [query]);

  return (
    <View>
      <View style={{ flexDirection: 'row' }}>
        <TextInput
          style={styles.textInput}
          placeholder={props.placeholder}
          value={labelSuggest ? labelSuggest : labelMap ? labelMap : null}
          onChangeText={(value) => setQuery(value)}
          autoCorrect={false}
          onFocus={() => {
            setPoint(null);
            getNameFromCoords(null);
            setLabelSuggest(null);
            setQuery(null);
            props.callback(null);
          }}
        />
        <TouchableOpacity onPress={() => setShowModal(true)}>
          <FontAwesome5 name="map-marked-alt" size={24} color="#32CD32" />
        </TouchableOpacity>
      </View>
      {response != null ? (
        <View style={styles.suggest}>
          {response.data.result.map((item) => {
            return (
              <View key={item.userData.id} style={{ marginVertical: 5 }}>
                <TouchableOpacity
                  onPress={() => {
                    item.userData.region.length > 0
                      ? setLabelSuggest(`${item.userData.suggestFirstRow}, ${item.userData.region}`)
                      : setLabelSuggest(item.userData.suggestFirstRow);
                    //console.log(item.userData)
                    setPoint({
                      longitude: item.userData.longitude,
                      latitude: item.userData.latitude,
                    });
                    props.callback({
                      name:
                        item.userData.region.length > 0 ? `${item.userData.suggestFirstRow}, ${item.userData.region}` : item.userData.suggestFirstRow,
                      longitude: item.userData.longitude,
                      latitude: item.userData.latitude,
                    });
                    setQuery(null);
                    Keyboard.dismiss();
                  }}
                >
                  <Text
                    style={{
                      fontSize: 15,
                      fontWeight: 'bold',
                    }}
                  >
                    {item.userData.suggestFirstRow}
                  </Text>
                  <Text style={{ fontSize: 12 }}>{item.userData.suggestSecondRow}</Text>
                </TouchableOpacity>
              </View>
            );
          })}
        </View>
      ) : null}

      <Modal visible={showModal} animationType="slide" transparent={true}>
        <View style={styles.modalContainer}>
          <MapView
            style={{
              width: Dimensions.get('window').width,
              height: Dimensions.get('window').height / 1.2,
            }}
            initialRegion={{
              latitude: point ? parseFloat(point.latitude) : parseFloat(props.initLatitude),
              longitude: point ? parseFloat(point.longitude) : parseFloat(props.initLongitude),
              latitudeDelta: props.initLatDelta ? props.initLatDelta : 0.1,
              longitudeDelta: props.initLonDelta ? props.initLonDelta : 0.1,
            }}
            onPress={async (event) => {
              setLabelSuggest(null);
              setPoint(event.nativeEvent.coordinate);
              setLabelMap(await getNameFromCoords(event.nativeEvent.coordinate));
            }}
          >
            {point ? (
              <Marker
                coordinate={{
                  latitude: point.latitude,
                  longitude: point.longitude,
                }}
              />
            ) : null}
          </MapView>
          <View
            style={{
              ...styles.componentContainer,
              width: Dimensions.get('window').width,
              justifyContent: 'space-evenly',
            }}
          >
            <TouchableOpacity onPress={() => setShowModal(false)}>
              <Entypo name="cross" size={50} color="red" />
            </TouchableOpacity>
            {point ? (
              <TouchableOpacity
                onPress={() => {
                  //wait until reverse geo returned result
                  let counter = 0;
                  let unsubscribe = setInterval(() => {
                    if (labelMap) {
                      props.callback({
                        name: labelMap,
                        longitude: point.longitude,
                        latitude: point.latitude,
                      });
                      clearTimeout(unsubscribe);
                      setShowModal(false);
                    }
                    //make sure it doesnt end up in infinite loop
                    if (++counter == 10) {
                      clearTimeout(unsubscribe);
                      //TODO: show error to user
                    }
                  }, 100);
                }}
              >
                <AntDesign name="checkcircleo" size={50} color="#32CD32" />
              </TouchableOpacity>
            ) : null}
          </View>
        </View>
      </Modal>
    </View>
  );
};

const data = {};

const styles = StyleSheet.create({
  componentContainer: {
    flexDirection: 'row',
    marginTop: 20,
    marginHorizontal: 40,
  },
  modalContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'rgba(52, 52, 52, 0.8)',
  },
  input: {
    backgroundColor: '#F5FFFA',
    width: Dimensions.get('window').width,
    fontSize: 20,
    paddingVertical: 5,
    paddingHorizontal: 10,
  },
  textInput: {
    borderBottomWidth: 2,
    fontSize: 20,
    flex: 1,
    marginRight: 5,
  },
  suggest: {
    borderBottomWidth: 2,
    borderLeftWidth: 2,
    borderRightWidth: 2,
    borderColor: '#32CD32',
    backgroundColor: '#F8F8FF',
    paddingHorizontal: 10,
    borderBottomLeftRadius: 20,
    borderBottomRightRadius: 20,
  },
});

export default ModalMapSuggestComponent;
