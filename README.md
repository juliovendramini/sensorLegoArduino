# sensorLegoArduino
Código base para transformar um arduino em um sensor de Lego Ev3 personalizado.
Recomendo utilizar o arduino pro mini, ou o nano, pois são pequenos. Mas para testes de bancada o UNO também funciona. Todos esses três utilizando o mesmo microcontrolador, então o codigo funcionará do mesmo jeito.

O que é necessário para conseguir utilizar essa biblioteca e montar o sensor no Lego EV3?
* Conhecimento de arduino
* Alicate para fazer o conector do EV3 e os conectores, nesse link da pra comprar ambos: https://pt.aliexpress.com/item/1005004581597068.html, esse link é só um exemplo, procure valores mais baratos no aliexpress.
* Saber o basico de soldas para juntar tudo.


Se for usar o EV3 com o micropython, é só usar o modo LUMPDevice e acessar o modo pelos íncide da função read. Mais informações em: https://pybricks.com/ev3-micropython/iodevices.html

Se for utilizar em Bloco no EV3-G, precisa importar um bloco especial. Em breve criarei esse bloco para poder usarem e teremos mais informações.
