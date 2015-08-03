/*var options = JSON.parse(localStorage.getItem('options'));
if (options === null) 
  options = { "url" : "http://www.cita.lu/info_trafic/cameras/images/cccam_{id}.jpg"}; */

var CHUNK_SIZE = 1500;
var DOWNLOAD_TIMEOUT = 20000;

function sendBitmap(bitmap){
  var i = 0;
  var nextSize = bitmap.length-i > CHUNK_SIZE ? CHUNK_SIZE : bitmap.length-i;
  var sliced = bitmap.slice(i, i + nextSize);

  MessageQueue.sendAppMessage({"size": bitmap.length});

  var success = function(){
    if(i>=bitmap.length)
      return;
    i += nextSize;
    console.log(i + "/" + bitmap.length);
    nextSize = bitmap.length-i > CHUNK_SIZE ? CHUNK_SIZE : bitmap.length-i;
    sliced = bitmap.slice(i, i + nextSize);
    MessageQueue.sendAppMessage(
      {
      "index":i,
      "chunk":sliced
      },
      success,
      null
      );
  };

  MessageQueue.sendAppMessage(
  {
    "index":i,
    "chunk":sliced
    },
    success,
    null
  );
}

function convertImage(rgbaPixels, numComponents, width, height){

  var watch_info;
  if(Pebble.getActiveWatchInfo) {
    watch_info = Pebble.getActiveWatchInfo() || { 'platform' : 'aplite'};
  } else {
    watch_info = { 'platform' : 'aplite'};
  }

  var ratio = Math.min(144 / width,168 / height);
  var ratio = Math.min(ratio,1);

  
  var final_width = 144;
  var final_height = 168;
  
  /*var final_width = Math.floor(width * ratio);
  var final_height = Math.floor(height * ratio);*/
  var final_pixels = [];
  var bitmap = [];

  if(watch_info.platform === 'aplite') {
    var grey_pixels = greyScale(rgbaPixels, width, height, numComponents);
    ScaleRect(final_pixels, grey_pixels, width, height, final_width, final_height, 1);
    floydSteinberg(final_pixels, final_width, final_height, pebble_nearest_color_to_black_white);
    bitmap = toPBI(final_pixels, final_width, final_height);
  }
  else {
    ScaleRect(final_pixels, rgbaPixels, width, height, final_width, final_height, numComponents);
    floydSteinberg(final_pixels, final_width, final_height, pebble_nearest_color_to_pebble_palette);
    var png = generatePngForPebble(final_width, final_height, final_pixels);
    for(var i=0; i<png.length; i++){
      bitmap.push(png.charCodeAt(i));
    }
  }

  return bitmap;
}

function getPbiImage(url){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", url, true);
  xhr.responseType = "arraybuffer";
  xhr.onload = function() {
    if(xhr.status == 200 && xhr.response) {
      clearTimeout(xhrTimeout); // got response, no more need in timeout
      var data = new Uint8Array(xhr.response);
      var bitmap = [];
      for(var i=0; i<data.byteLength; i++) {
        bitmap.push(data[i]);
      }
      sendBitmap(bitmap);
    }
  };

  var xhrTimeout = setTimeout(function() {
    MessageQueue.sendAppMessage({"message":"Error : Timeout"}, null, null);
  }, DOWNLOAD_TIMEOUT);

  xhr.send(null);
}

function getGifImage(url){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", url, true);
  xhr.responseType = "arraybuffer";
  xhr.onload = function() {
    clearTimeout(xhrTimeout); // got response, no more need in timeout

    MessageQueue.sendAppMessage({"message":"Decoding image..."}, null, null);

    var data = new Uint8Array(xhr.response || xhr.mozResponseArrayBuffer);
    var gr = new GifReader(data);
    console.log("Gif size : "+ gr.width  +" " + gr.height);

    var pixels = [];
    gr.decodeAndBlitFrameRGBA(0, pixels);

    var bitmap = convertImage(pixels, 4, gr.width, gr.height);

    sendBitmap(bitmap);
  };

  var xhrTimeout = setTimeout(function() {
    MessageQueue.sendAppMessage({"message":"Error : Timeout"}, null, null);
  }, DOWNLOAD_TIMEOUT);

  xhr.send(null);
}

function getJpegImage(url){
  var j = new JpegImage();
  j.onload = function() {
    clearTimeout(xhrTimeout); // got response, no more need in timeout

    MessageQueue.sendAppMessage({"message":"Decoding image..."}, null, null);

    console.log("Jpeg size : " + j.width + "x" + j.height);

    var pixels = j.getData(j.width, j.height);

    var bitmap = convertImage(pixels, 3, j.width, j.height);

    sendBitmap(bitmap);    
  };

  var xhrTimeout = setTimeout(function() {
    MessageQueue.sendAppMessage({"message":"Error : Timeout"}, null, null);
  }, DOWNLOAD_TIMEOUT);

  try{
    j.load(url);
  }catch(e){
    console.log("Error : " + e);
  }
}

function getPngImage(url){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", url, true);
  xhr.responseType = "arraybuffer";
  xhr.onload = function() {
    clearTimeout(xhrTimeout); // got response, no more need in timeout

    MessageQueue.sendAppMessage({"message":"Decoding image..."}, null, null);

    var data = new Uint8Array(xhr.response || xhr.mozResponseArrayBuffer);

    var png     = new PNG(data);
    var width   = png.width;
    var height  = png.height;
    var palette = png.palette;
    var pixels  = png.decodePixels();
    var bitmap  = [];

    if(palette.length > 0){
      var png_arr = [];
      for(var i=0; i<pixels.length; i++) {
        png_arr.push(palette[3*pixels[i]+0] & 0xFF);
        png_arr.push(palette[3*pixels[i]+1] & 0xFF);
        png_arr.push(palette[3*pixels[i]+2] & 0xFF);
      }
      bitmap = convertImage(png_arr, 3, width, height);
    }
    else {
      var components = pixels.length /( width*height);
      bitmap = convertImage(pixels, components, width, height);
    }

    sendBitmap(bitmap);
  };

  var xhrTimeout = setTimeout(function() {
    MessageQueue.sendAppMessage({"message":"Error : Timeout"}, null, null);
  }, DOWNLOAD_TIMEOUT);

  xhr.send(null);
}

function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}

function getImage(url){
  console.log("Image URL : "+ url);
  MessageQueue.sendAppMessage({"message":"Downloading image..."}, null, null);

  if(endsWith(url, ".pbi")){
    getPbiImage(url);
  }
  else if(endsWith(url, ".gif") || endsWith(url, ".GIF")){
    getGifImage(url);
  }
  else if(endsWith(url, ".jpg") || endsWith(url, ".jpeg") || endsWith(url, ".JPG") || endsWith(url, ".JPEG")){
    getJpegImage(url);
  }
  else if(endsWith(url, ".png") || endsWith(url, ".PNG")){
    getPngImage(url);
  }
  else {
    getJpegImage(url);
  }
}

function parseIncidents(doc) {
  var incidents = {};
  var pos = 0;
  
  var i = 0;
  
  while (doc.indexOf("<li>", pos) != -1) {
    var start = doc.indexOf("<li>", pos);
    //console.log("next item pos=" + start);
    pos = start + 1;
    //var end = doc.indexOf("</li>", pos);
    //console.log("end item pos=" + end);
    var spanstart = doc.indexOf("<span", pos);
    //console.log("span start pos=" + spanstart);
    
    var item = doc.substring(start + 4, spanstart).trim();
    //console.log("item: " + item.substring(1, 5));
    var date = doc.substring(spanstart + 22, spanstart + 41);
    
    //console.log(encodeURI(item));
    incidents[(i+1000).toString()] =  encodeURI(item);
    incidents[(i+2000).toString()] = date;
    i++;
  }
  
  incidents.incidents_size = i;

  //console.log('Send message: ' + JSON.stringify(incidents));
  console.log('Sending message! Parsed incidents: ' + incidents.incidents_size);
  
  Pebble.sendAppMessage(incidents, //null, null);
    function(e) {
      //console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      //console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );
  
}

function parseTravelTimes(doc) {
  var traveltimes = {};
  var pos = 0;
  
  var i = 0;
  
  while (doc.indexOf("<Placemark ", pos) != -1) {
    var placemark_start = doc.indexOf("<Placemark ", pos);
    pos = placemark_start + 1;
    var placemark_end = doc.indexOf("</Placemark>", pos);
    
    var name_start = doc.indexOf("<name>", pos);
    var name_end = doc.indexOf("</name>", pos);
    var name = doc.substring(name_start + 6, name_end);
    
    var placemarktimes = "";
    
    var subpos = pos;
    while (doc.indexOf("<span", subpos) != -1 && doc.indexOf("<span", subpos) < placemark_end) {
      var span_start = doc.indexOf("<span", subpos);
      subpos = span_start + 1;
      var span_end = doc.indexOf("</span>", subpos);
      var placemarktime = doc.substring(span_start + 19, span_end);
      placemarktimes += placemarktime + "\n";
    }
    
    //console.log("name=" + name + ", times=" + placemarktimes);
    
    traveltimes[(i+3000).toString()] = name;
    traveltimes[(i+4000).toString()] = placemarktimes;
    i++;
  }
  
  traveltimes.traveltimes_size = i;

  //console.log('Send message: ' + JSON.stringify(incidents));
  console.log('Sending message! Parsed travel times: ' + traveltimes.traveltimes_size);
  
  Pebble.sendAppMessage(traveltimes, //null, null);
    function(e) {
      //console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      //console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );
  
  
}

function getIncidents() {
  var req = new XMLHttpRequest();
  req.onload = function(e) {

    //console.log("ready state change - readyState=" + req.readyState);
    
    if (req.readyState == 4 && req.status == 200) {
      //console.log("responseText: " + req.responseText.substring(1, 4));

      parseIncidents(req.responseText);
    }
  };
  
  req.open('GET', 'http://www.cita.lu/fr/infos-trafic/generate', true);
  console.log("Fetching http://www.cita.lu/fr/infos-trafic/generate...");
  req.send();
}


function getTravelTimes() {
  var req = new XMLHttpRequest();
  req.onload = function(e) {

    //console.log("ready state change - readyState=" + req.readyState);
    
    if (req.readyState == 4 && req.status == 200) {
      //console.log("responseText: " + req.responseText.substring(1, 4));

      parseTravelTimes(req.responseText);
    }
  };
  
  req.open('GET', 'http://cita.lu/kml/temps_parcours.kml', true);
  console.log("http://cita.lu/kml/temps_parcours.kml...");
  req.send();
}

Pebble.addEventListener("ready", function(e) {
  // console.log("Ready to go!");  
});

Pebble.addEventListener("appmessage", function(e) {
  console.log('Received message: ' + JSON.stringify(e.payload));
  if (e.payload.incidents_req) {
    getIncidents();
  } else if (e.payload.traveltimes_req) {
    getTravelTimes();
  } else if (e.payload.webcam) {
    var webcamurl = "http://www.cita.lu/info_trafic/cameras/images/cccam_{id}.jpg";
    console.log('Downloading webcam #' + e.payload.webcam);
    getImage(webcamurl.replace('{id}', e.payload.webcam));
  }
});

/* Pebble.addEventListener('showConfiguration', function(e) {
  var uri = 'http://later';
  Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    options = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('options', JSON.stringify(options));
    getImage(options.url);
  } 
});
*/
