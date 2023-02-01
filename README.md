# GAMES101
Course Homework for GAMES101.
## Development Environment
- Hardware
  - Device: MacBook Pro 2021 14''
  - SoC: M1 Pro (8 cores CPU + 14 cores GPU)
  - System: macOS Ventura 13.2
- Software
  - Homebrew: v3.6.18
  - CMake: v3.25.2
  - OpenCV: v4.7.0_1
  - Eigen: v3.40_1

<details id=1>
<summary><h2>Homework 1</h2></summary>

### Commands
```bash
./Rasterizer                    #Rotating around z-axis (by default)

./Rasterizer -x                 #Rotating around x-axis

./Rasterizer -y                 #Rotating around y-axis

./Rasterizer -r 20              #Rotating the triangle around z-axis by 20 degree
                                #Save it to output.png

./Rasterizer -r 20 image.png    #Rotating the triangle around z-axis by 20 degree
                                #Save it to image.png

./Rasterizer -r 20 -x image.png #Rotating the triangle around x-axis by 20 degree
                                #Save it to image.png
```
### Basic
Rotating around z-axis
![basic](./images/homework1-basic.gif)

### Bonus
Rotating around x-axis
![bonus-x](images/homework1-bonus-x.gif)

Rotating around y-axis
![bonus-y](./images/homework1-bonus-y.gif)

</details>

<details id=2>
<summary><h2>Homework 2</h2></summary>

### Basic
![basic](./images/homework2-basic.png)

</details>