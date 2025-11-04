<style>
    h1 {
        text-align: center;
        margin-bottom: 1rem;
    }
    article {
        align-content: center;
        justify-items: center;
    }
    section {
        display: flex;
        flex-direction: column;
        gap: 0.5rem;
    }
    p {
        max-width: 125ch;
    }
</style>
<article>
<h1>
Que hice?
</h1>
<section>
<p>
Probe el Modulo LCD 160x128, Y para esto hice una interfaz que me mostrara 
datos de un Sensor Efecto Hall Linear Encapsulado TO 92UA AH3503 LPN O.
</p>

<p>
Lo que necesitaba era ver en tiempo real los datos que me daba el sensor 
respecto a la cercanía de un Imán de Neodimio para poder comparar la efectividad
entre imanes.
</p>
<p>
Como también necesitaba probar funcionalidades incluí en la Interfaz el valor de brillo actual y la posibilidad de cambiar el brillo usando el Serial Monitor.
</p>
</section>
<h1>
Optimización
</h1>
<section>
<p>
En cuanto a la optimización, Lo principal es entender los distintos 
tipos de <strong>primitivos</strong>:
</p>

<br>

| Primitivo | Bytes | Rango Aproximado | Negativos |
| :---------: | :-----: | :---------: | :---------: |
| `int` | 2 bytes | -32,768 a 32,767 (16 bits en Arduino UNO) | Si |
| `unsigned int` | 2 bytes | 0 a 65,535 | No |
| `long (int32_t)` | 4 bytes | -2,147,483,648 a 2,147,483,647 | Si |
| `unsigned long (uint32_t)`| 4 bytes | 0 a 4,294,967,295 | No |
| `int8_t` | 1 byte | -128 a 127 | Si |
| `uint8_t` | 1 byte | 0 a 255 | No |
| `int16_t` | 2 bytes | -32,768 a 32,767 | Si |
| `uint16_t` | 2 bytes | 0 a 65,535 | No |
| `float` | 4 bytes | ≈ ±3.4×10³⁸ | Si |

<br>

<p>
También hay que tener en cuenta que los <code>float</code> siempre operan en 32 bits por lo que
es mas lento.

Una alternativa mas ligera a una variable <code>float</code> seria reemplazar el valor por un entero y luego usar <strong>aritmética entera escalada</strong> es decir, usar enteros que representen decimales multiplicados por un factor (<i>técnica de fixed point arithmetic</i>).
</p>
<p>
<strong>Ejemplo:</strong>

Si tengo <code>alpha = 0.05;</code>, se podia representar como <code>alpha_scaled = 5;</code> y considerar que vale <code>5 / 100</code>

Lo que cambiaría <code type='cpp'>filtrado = alpha * lectura + (1 - alpha) * filtrado</code> 

a

<code type='cpp'>filtrado = ( (alpha_scaled * lectura) + ((100 - alpha_scaled) * filtrado) / 100) </code>

Con esto todas las variables podrían ser <code>int16_t o uint16_t</code>

Lo que eliminaría cálculos en coma flotante y por ende se ahorraría <strong>RAM</strong> y <strong>ciclos de CPU</strong>
</p>

</section>
<section>
<h1>
Porque no se usa <code>analogWriteResolution()</code>?
</h1>
<p>
Esta función solo existe en microcontroladores avanzados como el Arduino Leonardo, Due o ESP32.

Sirve para definir la resolución de salida PWM o DAC, es decir, cuantos bits usa la función <code>analogWrite()</code>.

Por defecto, <code>analogWrite()</code> trabaja en 8 bits (0-255)

Pero usando <code>analogWriteResolution()</code> le defino al microcontrolador que acepte valores de 0 a 1023 para escribir PWM (10 bits)

Esto permite que la salida sea mas precisa y suave, util para generar señales mas finas.
</p>
</section>
</article>
<!-- Me aburrí de hacer HTML que pereza  -->

# Qué hace el filtro?

```cpp
filtrado = alpha * lectura + (1 - alpha) * filtrado;
```

Es un **filtro paso bajo exponencial** (*Exponencial Moving Average*).
Su función es **suavizar** la señal del sensor para eliminar ruido, sin hacer lenta la operación.