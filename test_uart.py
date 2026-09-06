import urllib.request
url = "https://raw.githubusercontent.com/esphome/esphome/dev/esphome/components/uart/uart.h"
try:
    content = urllib.request.urlopen(url).read().decode("utf-8")
    if "load_settings" in content:
        print("load_settings found in uart.h")
    else:
        print("load_settings NOT found in uart.h")
except Exception as e:
    print(e)
