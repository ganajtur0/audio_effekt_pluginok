#!/bin/bash

cp ./manifest.ttl ./delay.ttl ./delay.so $HOME/.lv2/delay.lv2/ &&
	jalv.gtk3 https://github.com/ganajtur0/pluginok/delay
