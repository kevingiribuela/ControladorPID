# ControladorPID
 Controlador PID desarrollado en 4to año de la carrera

## Descripción
Este diseño se realizó basandose en los principios del control robusto. El objetivo es realizar el control automático de una planta conocida pero cuyos parámetros pueden variar en función de agentes externos como lo son la temperatura, la humedad, etc. Es por eso que el controlador debe no solo controlar a la planta nominal sino que también a la familia de plantas involucradas en el modelo. 

La planta se trata de una serie de filtros pasabajos en cascada implementados con amplificadores operacionales, resistencias y capacitores. Los archivos correspondientes al diseño de la placa se encuentran en la presente documentación.

La placa es capaz de realizar una identificación de la planta a partir de un escalón a su entrada y enviar los datos en tiempo real a MATLAB para su procesamiento de datos y posterior modelado a partir de un modelo de primer orden. 

<img src="https://github.com/kevingiribuela/ControladorPID/blob/main/placa2.jpg?raw=true">
