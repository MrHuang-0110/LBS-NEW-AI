
task0_finished = False

xRound = 0
yRound = 0
Brigntenss= 0
L_Y = 0
R_X = 0

        
def task1():
    while True:
        _motor.stop_module(1,1)
        _motor.stop_module(5,1)
        if _key.key_remote('X','press')==1:
            _motor.run_power(1,100,0)
        elif _key.key_remote('B','press')==1:
            _motor.run_power(1,-100,0)
        else:
            _motor.stop(1)
        if _key.key_remote('Y','press')==1:
            _motor.run_power(5,100,0)
        elif _key.key_remote('A','press')==1:
            _motor.run_power(5,-100,0)
        else:
            _motor.stop(5)
        _os.sleep_s(0.001)

def task2():
    global xRound
    global yRound
    global Brigntenss
    _matrix.clear()
    while True:
        xRound = _random.randint(0,7)
        yRound = _random.randint(0,9)
        Brigntenss = _random.randint(0,100)
        _matrix.set_pixel_brightness(xRound,yRound,Brigntenss)
        _os.sleep_s(0.01)

def task3():
    global L_Y, R_X, last_left_power, last_right_power   
    # 新增：记录上一次的电机功率状态
    last_left_power = 0
    last_right_power = 0
    _motor.pair(0,4)
    while True:
        _motor.mov_set_stop_module(1)
        L_Y = _key.key_remote('left','y')
        R_X = _key.key_remote('right','x')
        if (L_Y == 0 and R_X == 0):
            _motor.mov_stop()  
            is_moving = False  
            last_left_power = 0
            last_right_power = 0            
        elif (L_Y == 5 and R_X == 5):
            _motor.mov_stop()  
            is_moving = False  
            last_left_power = 0
            last_right_power = 0
        else:
            is_moving = True 
            base_speed = (L_Y - 5) * 25  
            turn_speed = (5 - R_X) * 25 
            deadzone = 2  
            if abs(base_speed) < deadzone:
                base_speed = 0
            if abs(turn_speed) < deadzone:
                turn_speed = 0
            left_power = base_speed - turn_speed
            right_power = base_speed + turn_speed
            left_power = max(-100, min(100, left_power))
            right_power = max(-100, min(100, right_power))
            if left_power != last_left_power or right_power != last_right_power:
                _motor.mov_power(left_power, right_power,0)
                last_left_power = left_power
                last_right_power = right_power
        _os.sleep_s(0.001)  

_thread.start_new_thread(task3, ())          
_thread.start_new_thread(task2, ())
_thread.start_new_thread(task1, ())

while not task0_finished:
    _os.sleep_s(0.1)
