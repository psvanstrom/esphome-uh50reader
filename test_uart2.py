import urllib.request
url = "https://raw.githubusercontent.com/esphome/esphome/dev/esphome/components/uart/uart_component.h"
try:
    content = urllib.request.urlopen(url).read().decode("utf-8")
    if "load_settings" in content:
        print("load_settings found in uart_component.h")
    else:
        print("load_settings NOT found in uart_component.h")
except Exception as e:
    print(e)
