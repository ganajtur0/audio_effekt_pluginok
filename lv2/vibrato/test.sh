#!/bin/bash

cp ./manifest.ttl ./vibrato.ttl ./vibrato.so $HOME/.lv2/vibrato.lv2/ &&
	jalv.gtk3 https://github.com/ganajtur0/pluginok/vibrato
