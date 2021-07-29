/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package network;

import java.util.ArrayList;
import java.util.Arrays;

/**
 *
 * @author klive
 */
public class CRC16 {
    
    /**
     * ћетод получени€ CRC16 из массива байт 
     * @param inputArray сообщение в виде массива байт
     * @return crc16 - контрольна€ сумма
     */
    public static byte[] getCrc(byte[] inputArray) {
        int len = inputArray.length;
        // ”становка начального значени€ CRC
        int crc16 = 0xFFFF;
        for (int i = 0; i < len; i++) {
            // XOR 8 битов данных с младшими 8 битами 16-битного CRC
            crc16 = ((crc16 & 0xFF00) | (crc16 & 0x00FF) ^ (inputArray[i] & 0xFF));
            //перебор со сдвигом 8 битов CRC
            for (int j = 0; j < 8; j++) {
                // —двиг содержимого CRC на один бит вправо, 
                //предварительна€ проверка значени€ данного бита 
                if ((crc16 & 0x0001) > 0) {
                    //если сдвигаемый бит = 1, то сдвиг на 1 и XOR с полиномом 0xA001
                    crc16 = crc16 >> 1;
                    crc16 = crc16 ^ 0xA001;
                } else
                    //если сдвигаемый бит = 0 - просто с сдвиг)
                    crc16 = crc16 >> 1;
            }
        }
        return intToBytes(crc16);
    }
    
    /**
     * ћетод добавлени€ CRC16 к массиву байт (также генерирует CRC16)
     * @param messageArray сообщение в виде масса байт без CRC
     * @return messageAndCrcArray - массив байт c CRC
     */
    public static byte[] addCrc(byte [] messageArray){
        //получение crc дл€ исходного массива
        byte [] crc16 = getCrc(messageArray);
        //объединение двух массивов
        byte[] messageAndCrcArray = Arrays.copyOf(messageArray, messageArray.length + crc16.length);
        System.arraycopy(crc16, 0, messageAndCrcArray, messageArray.length, crc16.length);
        return messageAndCrcArray;
    }

    /**
     * ћетод удалени€ CRC16 из массива байт
     * @param inputArray сообщение в виде массива байт с CRC
     * @return arrayWithoutCRC - сообщение без CRC
     */
    public static byte[] removeCrc (byte [] inputArray){
        int arraySize = inputArray.length;
        try {
        //выделение массива —ообщени€ из массива с —ообщением+CRC16
        inputArray = Arrays.copyOf(inputArray, arraySize-2);
        //сравнение полученного CRC16 с исходным
        } catch (Exception e) {
            System.out.println("Ќекорректное входное сообщение с CRC: " + e);
        }
            return inputArray;
    }
    
    /**
     * ћетод преобразовани€ CRC16 типа int в массив байт
     * @param intCRC16 CRC16 в формате int
     * @return byteCRC16 - CRC16 в формате массива байт
     */
    private static byte[] intToBytes(int intCRC16)  {
        byte[] byteCRC16 = new byte[2];
        byteCRC16[1] =  (byte) ((intCRC16>>8) & 0xFF);
        byteCRC16[0] =  (byte) (intCRC16 & 0xFF);
        return byteCRC16;
    }
    
    /**
     * ћетод проверки на совпадение CRC16 у поступившего сообщени€
     * @param inputArray сообщение в виде массива байт с CRC
     * @return true - если CRC совпал; false - если нет
     */
    public static boolean isCrcMatch(byte [] inputArray){
        int arraySize = inputArray.length;
        //получение байт-массива CRC16 из байт-массива —ообщени€+CRC16
        byte[] inputCrc = new byte[2];
        inputCrc[0] =  inputArray [arraySize - 2];
        inputCrc[1] =  inputArray [arraySize - 1];
        //выделение массива —ообщени€ из массива с —ообщением+CRC
        inputArray = removeCrc(inputArray);
        //сравнение полученного CRC16 с исходным
        byte [] realCrc = getCrc(inputArray);
        return Arrays.equals(inputCrc, realCrc);
    }
}
