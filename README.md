# Sekiro FOV changer
Either run directly and input the FOV, or pass the FOV as a command line argument with the syntax ```sekirofov <fov>```.

FOV is given in vertical degrees. The default is 43.

**Comparison screenshots**: https://imgur.com/a/gRsg3Jd

* To convert 4:3 based horizontal FOV (e.g. Source Engine) to vertical FOV:  
```atan(tan(fov * pi/360) * 3/4) * 360/pi```

* To convert horizontal FOV (e.g. Overwatch) to vertical FOV:  
```atan(tan(fov * pi/360) * ResY/ResX) * 360/pi```
