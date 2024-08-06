```
g++ -std=c++17 -o client client.cpp pic_send.cpp -I . -ltdpool -lpthread `pkg-config --cflags --libs opencv4`
```