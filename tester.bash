#!/bin/bash

for N in {1..50}
do
    ruby tester.rb &
done
wait