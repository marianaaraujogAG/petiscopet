**Petisco Pet**

A tecnologia não é mais um item de exclusividade das empresas e cada vez mais está se tornando um item de necessidade no dia a dia das pessoas, trazendo praticidade, conforto e sofisticação. Esses são alguns dos vários benefícios dos sistemas de automação residencial, vale destacar, segundo Wortmeyer, Freitas e Cardoso (2005) que a comodidade é dada pelo fato de, não ter a necessidade de abrir a porta da casa durante seu descanso assistindo televisão, por exemplo, já que tudo pode ser monitorado através de dispositivos eletrônicos.

Cada vez mais as famílias estão adquirindo pets porém nem sempre conseguem estar em suas casas para sempre alimenta-los fazendo com que eles passem longos períodos sozinhos, seja por questões de trabalho ou de imprevistos os pets não podem ficar sem alimentação.

Sendo assim, um protótipo para abastecer e monitorar o recipiente de comida do seu animal de estimação, tendo em vista que a alimentação no nosso pet é tão importnate quanto a nossa, esse sistema vem para ajudar familias que nem sempre estão presentes 100% do tempo para alimentar seus animais.

O projeto foi desenvolvido com a Placa Uno Arduino + Cabo USB e com os equipamentos abaixo:


**-** **Protoboard**


**-** **Módulo Relógio de Tempo Real RTC - DS3231**


**-** **Módulo Relé 1 Canal 12V**


**-** **Fonte de Alimentação Chaveada 5V 2A**


**-** **Motor DC 12V**


**-** **Jumper para Protoboard Macho-Fêmea**


**-** **Sensor Ultrassônico de Distância HC-SR04**


**-** **Shield Wifi para Arduino**


**-** **Dispenser de Parede Porta Alimentos**


**-** **Protocolo MQTT**


**Métodos**


O Arduino Uno conectado a um broker via protocolo MQTT, ao módulo relógio, módulo relé 12V. O motor estará conectado para fazer com que a comida do pet caia. Quando o animal já terminou de comer sua comida e o horário programado para a comida cair chegar, é enviada uma mensagem ao broker para iniciar o processo e então o módulo relé é acionado para que o motor comece a girar. O sensor de presença identifica se o animal já se alimentou e envia uma mensagem ao broker. Também haverá mensagens que serão enviadas pro meio de uma API sobre a quantidade de vezes em que o recipiente foi preenchido.
