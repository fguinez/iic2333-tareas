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
- Con el filtro activado y el visor de paquetes capturados vacío, realizar la conexión a la página del curso a través de su navegador web de preferencia. Debiesen aparecer 6 paquetes capturados. ¿Qué es lo que está ocurriendo? ¿A qué corresponden estos paquetes?
    - **Respuesta:** 
- Si espera unos momentos sin hacer nada en su navegador comenzará a recibir paquetes de tipo TCP Keepalive. ¿Qué significan estos paquetes? ¿Por qué se envían?
    - **Respuesta:** Son paquetes sin datos con el flag `ACK` activado, el recibirlos significa que la conexión con el servidor sigue funcionando correctamente. En general, estos paquetes se envían por 2 razones:
        1. Saber si la conexión sigue funcionando
        2. Evitar una desconexión por inactividad

&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; _Fuente: [The Linux Document Project](https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/overview.html)_

- Al navegar por la plataforma, observarán que Wireshark detecta paquetes del protocolo TCP de tipo RST. ¿A qué se deben estos paquetes? ¿Qué está ocurriendo? Creen un filtro de Wireshark de forma que solo muestre estos paquetes. ¿Cuál es este filtro?
    - **Respuesta:** 
    Para visualizar estos paquetes debemos usar el filtro `tcp.flags.reset == 1`.
- Expliquen qué es el protocolo TSL y para qué se usa. Describan además el Handshake Protocol de TSL. ¿Tiene sentido que aparezcan paquetes de este protocolo al conectarse a la plataforma SIDING? ¿Por qué?
    - **Respuesta:** 

## Caso 3: Red Local y Diagnóstico de Red
- Determinen las IPs públicas de sus redes locales. En consola ejecuten un test traceroute desde uno de sus equipos a la IP pública del compañero. Observen lo que ocurre en Wireshark. ¿Qué está pasando? ¿En qué consiste este test?
    - **Respuesta:** 
- Describan qué es el protocolo ICMP y para qué se usa generalmente.
    - **Respuesta:** 
- Si no utilizan ningún filtro, cada cierto tiempo van a recibir un paquete de protocolo ARP. ¿Cuál es la finalidad de este protocolo?
    - **Respuesta:** 
- Indiquen utilizando Wireshark si el servicio DHCP está habilitado en sus redes locales. Esto lo pueden compro- bar observando lo que ocurre en Wireshark cuando se conectan a sus redes locales. Describan lo que realizaron para llegar a su respuesta.
    - **Respuesta:** 
- En consola ejecuten un test nslookup hacia la IP de la plataforma SIDING. ¿Qué está ocurriendo? ¿Qué objetivo tiene este test?
    - **Respuesta:** 