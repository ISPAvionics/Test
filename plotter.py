from vpython import *
import serial
import math

# 修改成你的 COM port
ser = serial.Serial("COM7", 115200)
scene = canvas(title="MPU6050 3D Visualization", width=800, height=600, background=color.white)

# 建立一個代表飛機的物件
aircraft = box(length=1, height=0.2, width=0.5, color=color.red)

def set_orientation(roll, pitch, yaw):
    # 角度轉弧度
    roll = math.radians(roll)
    pitch = math.radians(pitch)
    yaw = math.radians(yaw)
    
    # 轉成旋轉矩陣
    # ZYX 旋轉順序
    Rz = [[math.cos(yaw), -math.sin(yaw), 0],
          [math.sin(yaw),  math.cos(yaw), 0],
          [0, 0, 1]]
    Ry = [[math.cos(pitch), 0, math.sin(pitch)],
          [0, 1, 0],
          [-math.sin(pitch), 0, math.cos(pitch)]]
    Rx = [[1,0,0],
          [0, math.cos(roll), -math.sin(roll)],
          [0, math.sin(roll),  math.cos(roll)]]
    
    # 矩陣相乘 R = Rz * Ry * Rx
    def mat_mult(A,B):
        return [[sum(A[i][k]*B[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
    
    R = mat_mult(Rz, mat_mult(Ry, Rx))
    
    # 設定 aircraft 方向
    aircraft.axis = vector(R[0][0], R[1][0], R[2][0])
    aircraft.up   = vector(R[0][2], R[1][2], R[2][2])

while True:
    try:
        line = ser.readline().decode('utf-8').strip()
        if line:
            roll, pitch, yaw = map(float, line.split(','))
            set_orientation(roll, pitch, yaw)
    except Exception as e:
        print("Error:", e)
        continue
