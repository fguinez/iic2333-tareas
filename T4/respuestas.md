[comment]: <> (Usar dillinger.io para exportar a pdf)

# Tarea 4
_IIC2333 - Sistemas Operativos y Redes_
_**Francisco Guíñez y Pedro Rioja**_

## Caso 1: Servidor UDP
- Cree un filtro de forma que solo se capturen paquetes cuya IP de destino sea 255.255.255.255 y que el protocolo sea UDP. ¿Cuál es este filtro?
    - **Respuesta:** `udp && ip.dst == 255.255.255.255`
- ¿Cuál es el tamaño en bytes del paquete completo? ¿Cuántos de estos corresponden a UDP? ¿A qué se debe esta diferencia?
    - **Respuesta:** El tamaño del paquete completo es **74 bytes**, de los cuales **8 bytes corresponden a UDP**. Esta diferencia se debe a que los paquetes deben incluir más información aparte de UDP, en este caso tenemos 14 bytes para Ethernet, 20 bytes para IPv4 y 32 bytes para data.
- ¿Cuál es el mensaje que emite el servidor? ¿Cuál es su largo en bytes en el paquete?
    - **Respuesta:** El mensaje que emite es `Mi numero de la suerte es: 414` con un largo de **32 bytes**.

## Caso 2: Conexión a SIDING
- ¿Cuál es esta IP?
    - **Respuesta:** 146.155.4.17
- Crear un filtro en Wireshark, de forma que solo se capturen paquetes cuyo origen o destino es la IP de la plataforma SIDING. ¿Cuál es este filtro?
    - **Respuesta:** `ip.addr == 146.155.4.17`
- Con el filtro activado y el visor de paquetes capturados vacío, realizar la conexión a la plataforma SIDING a través de su navegador web de preferencia. Debiesen aparecer aproximadamente 6 paquetes capturados al inicio de la conexión. ¿Qué es lo que está ocurriendo? ¿A qué corresponden estos paquetes?
    - **Respuesta:** Lo que está ocurriendo es que se está estableciendo la conexión con el servidor por medio de un TCP _handshake_ protegido con TLS. Entonces, los paquetes corresponden al proceso mencionado con el siguiente detalle:
        1. TCP - Un paquete `SYN` desde el cliente al servidor
        2. TCP - Un paquete `SYN, ACK` del servidor al cliente
        3. TCP - Un paquete `ACK` del cliente al servidor
        4. TLS - Un paquete `Client Hello` del cliente al servidor
        5. TLS - Un paquete `Server Hello` con el certificado desde el servidor al cliente
        6. TLS - Un paquete del cliente al servidor con un mensaje de protocolo de enlace ya cifrado 
- Si espera unos momentos sin hacer nada en su navegador comenzará a recibir paquetes de tipo TCP Keepalive. ¿Qué significan estos paquetes? ¿Por qué se envían?
    - **Respuesta:** Son paquetes sin datos con el flag `ACK` activado, el recibirlos significa que la conexión con el servidor sigue funcionando correctamente. En general, estos paquetes se envían por 2 razones:
        1. Saber si la conexión sigue funcionando
        2. Evitar una desconexión por inactividad

&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; _Fuente: [The Linux Document Project](https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/overview.html)_

- Al navegar por la plataforma, observarán que Wireshark detecta paquetes del protocolo TCP de tipo RST. ¿A qué se deben estos paquetes? ¿Qué está ocurriendo? Creen un filtro de Wireshark de forma que solo muestre estos paquetes. ¿Cuál es este filtro?
    - **Respuesta:** Estos paquetes se deben a que ha habido un problema en la comunicación, por lo que se necesita RST para volver a intenter la obtención de información. Por lo tanto, dado que todos los paquetes de este tipo se envían del cliente al servidor, lo que está ocurriendo es que nosotros como cliente estamos haciendo un nuevo intento por conseguir información faltante para obtener la información de la página solicitada.
    Para visualizar estos paquetes debemos usar el filtro `tcp.flags.reset == 1`. Opcionalmente, si queremos asegurarnos de que los paquetes mostrados corresponden explusivamente a SIDING, podemos usar el filtro `ip.addr == 146.155.4.17 && tcp.flags.reset == 1`
- Expliquen qué es el protocolo TLS y para qué se usa. Describan además el _Handshake Protocol_ de TLS. ¿Tiene sentido que aparezcan paquetes de este protocolo al conectarse a la plataforma SIDING? ¿Por qué?
    - **Respuesta:** El _Transport Layer Security_ (TLS) es un protocolo que se encarga de que la comunicación sea segura por medio de la criptografía. Se utiliza ampliamente en las comunicaciones por internet, para por ejemplo la mensajería electrónica, servicios que requieren inicio de sesión, navegación web en general, incluso ha sido utilizado para crear servicios VPN.
    El _Handshake Protocol_ de TLS ocurre justo después de establecida la conexión (comunmente por medio de TCP, como en el caso de Siding) y resumidamente consiste en que:
        1. Cliente y servidor se saludan, proceso en el cual ambos aportan una serie de números pseudo-aleatorios para producir las llaves.
        2. Luego, el servidor envía un certificado al cliente, con esta información el cliente confirma la identidad del servidor
        3. El cliente procede a enviar el _premaster secret_ con un último grupo de números pseudo-aleatorios, el cual se encripta con la llave pública del servidor por lo que solo puede ser leído por él.
        4. Con los números pseudo-aleatorios generados, cliente y servidor generan las llaves de sesión.
        5. Se envían mutuamente mensajes _Finished_ ya encriptados con la llave de sesión.
    
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; Una vez hecho esto el servidor ofrece un certificado que el cliente debe verificar.
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; Posteriormente, cuando esa verificación se ha llevado a cabo, se genera una sesión.
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; Se crea una clave a través de la cual se intercambian datos a través de esa sesión.

&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; Por todo lo dicho anteriormente, *si tiene sentido* que aparezcan paquetes de este
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; protocolo al conectarse a la plataforma, porque permite cifrar las comunicaciones
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; entre los estudiantes y SIDING desde el comienzo, permitiendo comunicaciones
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; seguras.

## Caso 3: Red Local y Diagnóstico de Red
- Determinen las IPs públicas de sus redes locales. En consola ejecuten un test traceroute desde uno de sus equipos a la IP pública del compañero. Observen lo que ocurre en Wireshark. ¿Qué está pasando? ¿En qué consiste este test?
    - **Respuesta:** 
- Describan qué es el protocolo ICMP y para qué se usa generalmente.
    - **Respuesta:** 
- Si no utilizan ningún filtro, cada cierto tiempo van a recibir un paquete de protocolo ARP. ¿Cuál es la finalidad de este protocolo?
    - **Respuesta:** 
- Indiquen utilizando Wireshark si el servicio DHCP está habilitado en sus redes locales. Esto lo pueden compro- bar observando lo que ocurre en Wireshark cuando se conectan a sus redes locales. Describan lo que realizaron para llegar a su respuesta.
    - **Respuesta:** 
- En consola ejecuten un test nslookup hacia la IP de la plataforma SIDING. ¿Qué está ocurriendo? ¿Qué objetivo tiene este test?
    - **Respuesta:** 