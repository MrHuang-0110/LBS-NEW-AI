
SuDu_ = 0
DDianJi_ = 0


task1_finished = False

def customFunc11(sp, jd):
    global SuDu_, DDianJi_
    _motor.pair(0,1)
    _motor.mov_set_stop_module(1)
    _motor.reset_relative_position(0)
    SuDu_ = 0
    _os.resetTimer()
    _os.sleep_s(0.1)
    while not (_math.fabs(_motor.relative_angle(0)) > jd):
        if SuDu_ > sp - 1:
            SuDu_ = sp
        else:
            SuDu_ = (sp * _os.timer())
        _motor.mov_find_line_run(_gray.line(6, 2), _gray.line(6, 3), -1 * SuDu_, 0.1, 0.0075)
        _os.sleep_s(0.001)
    _motor.mov_stop()
    _os.sleep_s(0.1)
    
def task1():
    global task1_finished
    global SuDu_, DDianJi_
    customFunc11(70, 1000)
    task1_finished = True


_thread.start_new_thread(task1, ())

while not task1_finished:
    _os.sleep_s(0.1)

_os.clear_thread()