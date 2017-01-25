# linux-driverI2c
Driver para leer datos de la micro estación meteorológica Sparkfun Weather shield

El protocolo I2C es un bus tipo serial que es utilizado para comunicarse con dispositivos como EEPROMS y periféricos de un rango amplio -incluso existe el S;Bus, que puede considerarse como un subgrupo dentro de I2C -. El kernel de linux divide a I2C en 'Buses' y 'Dispositivos' para luego descoponerlo en 'algoritmos' y 'adaptadores' y los dispositivos se dividen a su vez en Drivers y clientes.
