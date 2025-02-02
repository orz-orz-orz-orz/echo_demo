# Echo cancellation
---
## 程式說明
通常，使用電腦時同時用麥克風錄音和喇叭放音時，會造成回授產生爆音的問題。
由於我們知道之前播放的訊號是什麼，只要能夠知道喇叭到麥克風之間的通道，
就可以從收到的訊號中扣掉回授的訊號，只留下想要的訊號。

實作上，參考的是 "Study of the General Kalman Filter for Echo Cancellation" (https://ieeexplore.ieee.org/document/6457439) 這篇文章中的方法。
使用式 (63)-(68) 的 order=1 的式子。

---

## 編譯和執行
直接專案目錄下使用 make 進行編譯即可：
```
make
```
或是使用支援 C++17 的 g++/clang++編譯:
```
g++ record.cpp -o record -O2 -Ieigen-3.4.0 -lm -std=c++17
```
直接執行 
```
./record
```
就會開始錄音了。

## 影片



https://github.com/user-attachments/assets/3282d5e6-1cff-4042-82b9-f5d7c48f38b4

---

## 限制
- 目前沒辦法完全解決爆音的問題，尤其是在喇叭放出的聲音比目標人聲更大的時候。
- miniaudio 由於不能控制幾點呼叫一次 callback。在我的 mac 上是 441 點呼叫一次 callback function，使得計算時間非常受限。
- 如果計算時間太長會有不連續的問題。
- 這邊是通過降低 order 數和通道參數的更新頻率來解決。
- 演算法無法保證穩定性，order 數太大會有不穩定的問題。
- 目前沒有處理 buffer 的問題，錄太久記憶體會爆掉。

--- 
## 使用到的第三方函式庫
- Eigen-3.4.0 用來做矩陣運算。
- miniaudio 用來錄音和放音。




