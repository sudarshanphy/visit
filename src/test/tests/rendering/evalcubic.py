# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  evalcubic.py
#
#  Tests:      mesh      - 3D structured, single domain
#              plots     - pseudocolor
#
#  Programmer: Cyrus Harrison
#  Date:       Mon Feb  7 15:52:37 PST 2011
#
#  Modifications:
#
# ----------------------------------------------------------------------------

#
# Example from:
# https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/tutorials/Scripting.html#animating-the-camera
#

def fly():
    # Do a pseudocolor plot of u.
    DeleteAllPlots()
    AddPlot('Pseudocolor', 'hardyglobal')
    AddOperator("Clip")
    c = ClipAttributes()
    c.funcType = c.Sphere  # Plane, Sphere
    c.center = (0, 0, 0)
    c.radius = 10
    c.sphereInverse = 1
    SetOperatorOptions(c)
    DrawPlots()
    # Create the control points for the views.
    c0 = View3DAttributes()
    c0.viewNormal = (0, 0, 1)
    c0.focus = (0, 0, 0)
    c0.viewUp = (0, 1, 0)
    c0.viewAngle = 30
    c0.parallelScale = 17.3205
    c0.nearPlane = 17.3205
    c0.farPlane = 81.9615
    c0.perspective = 1
    c1 = View3DAttributes()
    c1.viewNormal = (-0.499159, 0.475135, 0.724629)
    c1.focus = (0, 0, 0)
    c1.viewUp = (0.196284, 0.876524, -0.439521)
    c1.viewAngle = 30
    c1.parallelScale = 14.0932
    c1.nearPlane = 15.276
    c1.farPlane = 69.917
    c1.perspective = 1
    c2 = View3DAttributes()
    c2.viewNormal = (-0.522881, 0.831168, -0.189092)
    c2.focus = (0, 0, 0)
    c2.viewUp = (0.783763, 0.556011, 0.27671)
    c2.viewAngle = 30
    c2.parallelScale = 11.3107
    c2.nearPlane = 14.8914
    c2.farPlane = 59.5324
    c2.perspective = 1 
    c3 = View3DAttributes()
    c3.viewNormal = (-0.438771, 0.523661, -0.730246)
    c3.focus = (0, 0, 0)
    c3.viewUp = (-0.0199911, 0.80676, 0.590541)
    c3.viewAngle = 30
    c3.parallelScale = 8.28257
    c3.nearPlane = 3.5905
    c3.farPlane = 48.2315
    c3.perspective = 1
    c4 = View3DAttributes()
    c4.viewNormal = (0.286142, -0.342802, -0.894768)
    c4.focus = (0, 0, 0)
    c4.viewUp = (-0.0382056, 0.928989, -0.36813)
    c4.viewAngle = 30
    c4.parallelScale = 10.4152
    c4.nearPlane = 1.5495
    c4.farPlane = 56.1905
    c4.perspective = 1
    c5 = View3DAttributes()
    c5.viewNormal = (0.974296, -0.223599, -0.0274086)
    c5.focus = (0, 0, 0)
    c5.viewUp = (0.222245, 0.97394, -0.0452541)
    c5.viewAngle = 30
    c5.parallelScale = 1.1052
    c5.nearPlane = 24.1248
    c5.farPlane = 58.7658
    c5.perspective = 1
    c6 = c0
    # Create a tuple of camera values and x values. The x values are weights
    # that help to determine where in [0,1] the control points occur.
    cpts = (c0, c1, c2, c3, c4, c5, c6)
    x=[]
    for i in range(7):
        x = x + [float(i) / float(6.)]
    # Animate the camera. Note that we use the new built-in EvalCubicSpline
    # function which takes a t value from [0,1] a tuple of t values and a tuple
    # of control points. In this case, the control points are View3DAttributes
    # objects that we are using to animate the camera but they can be any object
    # that supports +, * operators.
    nsteps = 100
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        c = EvalCubicSpline(t, x, cpts)
        c.nearPlane = -34.461
        c.farPlane = 34.461
        SetView3D(c)
        if i == 0:
            Test("evalcubic_0")
        elif i == 24:
            Test("evalcubic_1")
        elif i == 49:
            Test("evalcubic_2")
        elif i == 74:
            Test("evalcubic_3")
        elif i == 99:
            Test("evalcubic_4")


OpenDatabase(silo_data_path("noise.silo"))

fly()
Exit()
