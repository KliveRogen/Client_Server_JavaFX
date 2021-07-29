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
     * ����� ��������� CRC16 �� ������� ���� 
     * @param inputArray ��������� � ���� ������� ����
     * @return crc16 - ����������� �����
     */
    public static byte[] getCrc(byte[] inputArray) {
        int len = inputArray.length;
        // ��������� ���������� �������� CRC
        int crc16 = 0xFFFF;
        for (int i = 0; i < len; i++) {
            // XOR 8 ����� ������ � �������� 8 ������ 16-������� CRC
            crc16 = ((crc16 & 0xFF00) | (crc16 & 0x00FF) ^ (inputArray[i] & 0xFF));
            //������� �� ������� 8 ����� CRC
            for (int j = 0; j < 8; j++) {
                // ����� ����������� CRC �� ���� ��� ������, 
                //��������������� �������� �������� ������� ���� 
                if ((crc16 & 0x0001) > 0) {
                    //���� ���������� ��� = 1, �� ����� �� 1 � XOR � ��������� 0xA001
                    crc16 = crc16 >> 1;
                    crc16 = crc16 ^ 0xA001;
                } else
                    //���� ���������� ��� = 0 - ������ � �����)
                    crc16 = crc16 >> 1;
            }
        }
        return intToBytes(crc16);
    }
    
    /**
     * ����� ���������� CRC16 � ������� ���� (����� ���������� CRC16)
     * @param messageArray ��������� � ���� ����� ���� ��� CRC
     * @return messageAndCrcArray - ������ ���� c CRC
     */
    public static byte[] addCrc(byte [] messageArray){
        //��������� crc ��� ��������� �������
        byte [] crc16 = getCrc(messageArray);
        //����������� ���� ��������
        byte[] messageAndCrcArray = Arrays.copyOf(messageArray, messageArray.length + crc16.length);
        System.arraycopy(crc16, 0, messageAndCrcArray, messageArray.length, crc16.length);
        return messageAndCrcArray;
    }

    /**
     * ����� �������� CRC16 �� ������� ����
     * @param inputArray ��������� � ���� ������� ���� � CRC
     * @return arrayWithoutCRC - ��������� ��� CRC
     */
    public static byte[] removeCrc (byte [] inputArray){
        int arraySize = inputArray.length;
        try {
        //��������� ������� ��������� �� ������� � ����������+CRC16
        inputArray = Arrays.copyOf(inputArray, arraySize-2);
        //��������� ����������� CRC16 � ��������
        } catch (Exception e) {
            System.out.println("������������ ������� ��������� � CRC: " + e);
        }
            return inputArray;
    }
    
    /**
     * ����� �������������� CRC16 ���� int � ������ ����
     * @param intCRC16 CRC16 � ������� int
     * @return byteCRC16 - CRC16 � ������� ������� ����
     */
    private static byte[] intToBytes(int intCRC16)  {
        byte[] byteCRC16 = new byte[2];
        byteCRC16[1] =  (byte) ((intCRC16>>8) & 0xFF);
        byteCRC16[0] =  (byte) (intCRC16 & 0xFF);
        return byteCRC16;
    }
    
    /**
     * ����� �������� �� ���������� CRC16 � ������������ ���������
     * @param inputArray ��������� � ���� ������� ���� � CRC
     * @return true - ���� CRC ������; false - ���� ���
     */
    public static boolean isCrcMatch(byte [] inputArray){
        int arraySize = inputArray.length;
        //��������� ����-������� CRC16 �� ����-������� ���������+CRC16
        byte[] inputCrc = new byte[2];
        inputCrc[0] =  inputArray [arraySize - 2];
        inputCrc[1] =  inputArray [arraySize - 1];
        //��������� ������� ��������� �� ������� � ����������+CRC
        inputArray = removeCrc(inputArray);
        //��������� ����������� CRC16 � ��������
        byte [] realCrc = getCrc(inputArray);
        return Arrays.equals(inputCrc, realCrc);
    }
}
