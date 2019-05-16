	  var wifiSSIDField = document.getElementById("ssid");
	  var wifiPasswordField = document.getElementById("pass");
	  var wifiNetworksListBox = document.getElementById("wifiNetworks");
	  var apModeListBox = document.getElementById("apMode");
	  var showPassCB = document.getElementById("showPassCB");
	  var rssiField = document.getElementById("rssiField");
	  var channelField = document.getElementById("channelField");
	  var authField = document.getElementById("authField");
	  var netParmDict = {};

	  var log = function(msg) {
	  	$('<span/>').html(msg + '\n').appendTo('#result')
	  };
	  
	  function updateSelectedNetwork() {
	  	var selectedIndex = wifiNetworksListBox.selectedIndex;
	  	if (selectedIndex > -1) {
	  		wifiSSIDField.value = wifiNetworksListBox.options[selectedIndex].text;
	  		netParms = wifiNetworksListBox.options[selectedIndex].value;
	  		var elems = netParms.split(",");
	  		if (elems.length == 3) {
	  			rssiField.value = elems[0];
	  			channelField.value = elems[1];
	  			authField.value = elems[2];
	  		}
	  	}
	  }

	  function scanWifi() {
	  	opt = document.createElement("option");
	  	wifiNetworksListBox.options.add(opt);
	  	opt.text = "Scanning...";

	  	$('#wifiNetworks').empty();
	  	opt = document.createElement("option");
	  	wifiNetworksListBox.options.add(opt);
	  	opt.text = "Scanning...";
	  	log('Scanning for WiFi networks...');
	  	$.ajax({
	  		url: '/rpc/WiFi.Scan',
	  		data: "",
	  		type: 'POST',
	  		success: function(data) {

	  			if ("results" in data) {
	  				log('Success.');
	  				resultSetDict = data["results"];
	  				rssList = [];
	  				networkDict = {};
	  				toolTipDict = {};
	  				for (var key in resultSetDict) {
	  					networkResultDict = resultSetDict[key];
	  					var rssi = null;
	  					var ssid = null;
	  					var channel = null;
	  					var authMode = null;
	  					for (var key in networkResultDict) {
	  						if (key == "rssi") {
	  							rssi = networkResultDict[key];
	  						}
	  						if (key == "ssid") {
	  							ssid = networkResultDict[key];
	  						}
	  						if (key == "channel") {
	  							channel = networkResultDict[key];
	  						}
	  						if (key == "auth") {
	  							authModeID = networkResultDict[key];
	  							if (authModeID == 0) {
	  								authMode = "Open";
	  							} else if (authModeID == 1) {
	  								authMode = "WEP";
	  							} else if (authModeID == 2) {
	  								authMode = "WPA_PSK";
	  							} else if (authModeID == 3) {
	  								authMode = "WPA2_PSK";
	  							} else if (authModeID == 4) {
	  								authMode = "WPA_WPA2_PSK";
	  							} else if (authModeID == 5) {
	  								authMode = "WPA2_ENTERPRISE";
	  							}
	  						}
	  						if (rssi && ssid && channel) {
	  							rssList.push(rssi);
	  							networkDict[rssi] = ssid;
	  							toolTipDict[rssi] = "RSSI: " + rssi + " dBm\nChannel: " + channel + "\nAuthorisation Mode: " + authMode;
	  							netParmDict[rssi] = rssi + "," + channel + "," + authMode;
	  						}
	  					}
	  				}
	  				//Sort so that the highest RXP occurs first in the list
	  				rssList.sort();
	  				$('#wifiNetworks').empty();
	  				for (var index in rssList) {
	  					thisRSSI = rssList[index];
	  					$('#wifiNetworks').append($("<option/>", {
	  						value: netParmDict[thisRSSI],
	  						text: networkDict[thisRSSI],
	  						title: toolTipDict[thisRSSI]
	  					}));
	  				}

	  				if( wifiNetworksListBox.length > 0 ) {
						wifiNetworksListBox.selectedIndex=0;	 
						log(""+wifiNetworksListBox.options[0].text+" SSID is the loudest."); 				
	  				}
					updateSelectedNetwork();
	  			}
	  		},
	  	});

	  }

	  wifiNetworksListBox.addEventListener("change", function() {
			updateSelectedNetwork();
	  });

	  showPassCB.addEventListener("click", function() {
	  	if (wifiPasswordField.type === "password") {
	  		wifiPasswordField.type = "text";
	  	} else {
	  		wifiPasswordField.type = "password";
	  	}
	  });

	  $('#save').on('click', function() {
		if( $('#ssid').val().length < 8 ) {
			alert("The WiFi SSID must be at least 8 characters long.");
			return;
		}
		if( $('#pass').val().length < 8 ) {
			if (confirm("Did you mean to leave the WiFi password field empty ?")) {
			    // Ok proceed
			} else {
			    return;
			}
		}  	
	  	
	  	log('Saving WiFi settings ...');

	  	var jsonStr = JSON.stringify({
	  		config: {
	  			user: {
	  				setup_mode: 0,
	  				wifi_mode: 1
	  			},
	  			wifi: {
	  				ap: {
	  					enable: false
	  				},
	  				sta: {
	  					enable: true,
	  					ssid: $('#ssid').val(),
	  					pass: $('#pass').val()
	  				}
	  			}
	  		}
	  	});
	  	if (apModeListBox.selectedIndex == 1) {
	  		jsonStr = JSON.stringify({
	  			config: {
	  				user: {
	  					setup_mode: 0,
	  					wifi_mode: 0
	  				},
	  				wifi: {
	  					sta: {
	  						enable: false
	  					},
	  					ap: {
	  						enable: true,
	  						ssid: $('#ssid').val(),
	  						pass: $('#pass').val()
	  					}
	  				}
	  			}
	  		});
	  	}
	  	if (apModeListBox.selectedIndex == 2) {
	  		jsonStr = JSON.stringify({
	  			config: {
	  				user: {
	  					setup_mode: 0,
	  					wifi_mode: 0
	  				},
	  				wifi: {
	  					sta: {
	  						enable: true,
	  						ssid: $('#ssid').val(),
	  						pass: $('#pass').val()
	  					},
	  					ap: {
	  						enable: true,
	  						ssid: $('#ssid').val()+"_AP",
	  						pass: $('#pass').val()
	  					}
	  				}
	  			}
	  		});
	  	}
	   $.ajax({
	  		url: '/rpc/Config.Set',
	  		data: jsonStr,
	  		type: 'POST',
	  		success: function(data) {
	  			log('WiFi configured.');
			  	$.ajax({
			  		url: '/rpc/WiFi.SetupComplete',
	  		  		data: "",
			  		type: 'POST',
			  		success: function(data) {
			  			log('Restarting ...');
			  		}
			  	});			
	  		},

	  	})
	  });

	  $('#scan').on('click', function() {
	  	scanWifi();
	  });		 
	  
