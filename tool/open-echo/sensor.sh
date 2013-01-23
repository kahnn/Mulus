#!/bin/sh
CPATH=.:./lib/echo.jar
CPATH=${CPATH}:./classes

java -classpath ${CPATH} sensor.SampleTemperatureSensor
