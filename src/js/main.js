var options = JSON.parse(localStorage.getItem('options'));
if (options === null) 
  options = { "url" : "http://www.cita.lu/info_trafic/cameras/images/cccam_42.jpg"};

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
  
  var i = 1000;
  
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
    
    incidents[i.toString()] = item;
    //incidents[(i+1000).toString()] = date;
    i++;
  }
  
  incidents.incidents_size = i - 1000;

  //console.log('Send message: ' + JSON.stringify(incidents));
  console.log('Send message! num messages=' + incidents.incidents_size);
  
  Pebble.sendAppMessage(incidents, null, null);
  /*
    function(e) {
      //console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      //console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );*/
  
  //console.log("exiting parseIncidents, transactionId=" + transactionId);
  
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
  console.log("calling cita.lu");
  req.send();
}

Pebble.addEventListener("ready", function(e) {
  // console.log("Ready to go!");  
});

Pebble.addEventListener("appmessage", function(e) {
  console.log('Received message: ' + JSON.stringify(e.payload));
  if (e.payload.incidents_req) {
    getIncidents();
  } else {
    getImage(options.url);
  }
});

Pebble.addEventListener('showConfiguration', function(e) {
  var uri = 'http://petitpepito.free.fr/config/imageviewer_config.html';
  Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    options = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('options', JSON.stringify(options));
    getImage(options.url);
  } 
});



