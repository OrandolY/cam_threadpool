```
g++ -std=c++17 -o server server.cpp catch_pics.cpp fun_pic.cpp -I . -ltdpool -lpthread `pkg-config --cflags --libs opencv4`
```