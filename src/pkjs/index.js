var Clay = require("@rebble/clay");
var clayConfig = require("./config");
var clay = new Clay(clayConfig);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function weatherCodeToCondition(code) {
  if (code === 0) return "Clear";
  if (code <= 2) return "Sunny";
  if (code === 3) return "Cloudy";
  if (code <= 48) return "Fog";
  if (code <= 55) return "Drizzle";
  if (code <= 57) return "Fz. Drizzle";
  if (code <= 65) return "Rain";
  if (code <= 67) return "Fz. Rain";
  if (code <= 75) return "Snow";
  if (code <= 77) return "Snow Grains";
  if (code <= 82) return "Showers";
  if (code <= 86) return "Snow Shwrs";
  if (code === 95) return "T-Storm";
  if (code <= 99) return "T-Storm";
  return "Unknown";
}

function locationSuccess(pos) {
  var url =
    "https://api.open-meteo.com/v1/forecast?" +
    "latitude=" +
    pos.coords.latitude +
    "&longitude=" +
    pos.coords.longitude +
    "&current=temperature_2m,weather_code" +
    "&daily=weather_code,sunrise,sunset" +
    "&timezone=auto";

  xhrRequest(url, "GET", function (responseText) {
    var json = JSON.parse(responseText);

    var temperature = Math.round(json.current.temperature_2m);
    var conditions = weatherCodeToCondition(json.current.weather_code);

    var dictionary = {
      TEMPERATURE: temperature,
      CONDITIONS: conditions
    };

    var codes = json.daily.weather_code;
    for (var i = 0; i < 7 && i < codes.length; i++) {
      dictionary["WEATHER_DAY_" + i] = codes[i];
    }

    var sunrise = Math.floor(new Date(json.daily.sunrise[0]).getTime() / 1000);
    var sunset = Math.floor(new Date(json.daily.sunset[0]).getTime() / 1000);
    dictionary["SUNRISE"] = sunrise;
    dictionary["SUNSET"] = sunset;

    Pebble.sendAppMessage(
      dictionary,
      function (e) {
        console.log("codes:", JSON.stringify(codes));
        console.log("Weather info sent!");
      },
      function (e) {
        console.log("Error sending weather info!");
      }
    );
  });
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {
    timeout: 15000,
    maximumAge: 60000
  });
}

Pebble.addEventListener("ready", function (e) {
  console.log("PebbleKit JS ready!");
  getWeather();
});

Pebble.addEventListener("appmessage", function (e) {
  console.log("AppMessage received!");
  if (e.payload["REQUEST_WEATHER"]) {
    getWeather();
  }
});
