    /******************** (C) COPYRIGHT  风驰iCreate嵌入式开发工作室 ***************************
        |--------------------|
        |  USART1_RX-PA4     |
        |  USART1_TX-PA5     |
        |--------------------|
    ****************************************************************************************/
    
    #include "uart.h"

    void uart_conf(void)
    {
        unsigned int baud_div=0;
        
        UART1_CR1 = (0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0); 
        /*1位起始位 8位数据位 结束位由CR3设置 不使用奇偶校验 不使能奇偶校验中断*/
    /*
    	CR1[5]:UARTD	UART禁用，0->使能UART
    	CR1[4]:M字长	0->一个起始，8个数据，n个停止(取决于CR3的STOP[1:0])；
    					1->一个起始，8个数据，一个停止；
    	CR[3]:WAKE		唤醒方法：	0->被空闲总线唤醒；
    								1->被地址标记唤醒；
    	CR[2]:PCEN		奇偶校验控制使能	1->使能校验；
    	CR[1]:PS		校验选择	0->偶校验	1->奇校验；
    	CR[0]:PIEN		校验中断使能 1->当USART_SR中的PE为1时，产生USART中断；
    */	
    	
        UART1_CR2 = (0<<7)|(0<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2); 
        /*使能发送和接收 接收中断使能 禁止发送中断*/
    /*	
    	CR2[7]:TIEN		发送中断使能	1->当USART_SR中的TXE为1时，产生USART中断；
    	CR2[6]:TCIEN	发送完成中断使能	1->当USART_SR中的TC为1时，产生USART中断；
    	CR2[5]:RIEN		接收中断使能	1->当USART_SR中的OR或者RXNE为1时，产生USART中断；
    	CR2[4]:ILIEN	IDLE中断使能	1->当USART_SR中的IDLE为1时，产生USART中断；
    	CR2[3]:TEN		发送使能；
    	CR2[2]:REN		接收使能；
    */	
    	
        UART1_CR3 = (0<<6)|(0<<4)|(0<<3); /*设置1位停止位 不使能SCLK*/    
    /*
    	CR3[6]:LINEN	LIN模式使能；
    	CR3[5:4]:STOP[1:0]	停止位 	00->一个停止位；
    								10->两个停止位；
    								11->1.5个停止位；
    	CR[3]:CLKEN		时钟使能 SCK引脚使能；
    */	
        /* CR4:LIN 控制寄存器		CR5:智能卡控制*/
        UART1_CR5 = (0<<2)|(0<<1);     
        /*使用智能卡模式需要设置的，基本上保持默认就行了 */ 
        
        /*设置波特率*/
        baud_div =HSIClockFreq/BaudRate;  /*求出分频因子*/
        UART1_BRR2 = baud_div & 0x0f;
        UART1_BRR2 |= ((baud_div & 0xf000) >> 8);
        UART1_BRR1 = ((baud_div & 0x0ff0) >> 4);    /*先给BRR2赋值 最后再设置BRR1*/
        
        UART1_CR1 |= (0<<5);         /*使能UART*/
    }
    
    /**************************************************************************
     * 函数名：UART1_SendByte
     * 描述  ：uart发送一个字节
     * 输入  ：u8 data
     * 举例  ：UART1_SendByte('a')
     *************************************************************************/
    void UART1_SendByte(u8 data)
    {
        UART1_DR=data;
       /* Loop until the end of transmission */
       while (!(UART1_SR & UART1_FLAG_TXE));
    }
    
    /**********************************************************************************
     * 函数名：UART1_SendString
     * 描述  ：uart发送字符串
     * 输入  ：u8* Data,u16 len
     * 举例  ：UART1_SendString("iCreate STM8开发板",sizeof("iCreate STM8开发板"))
     **********************************************************************************/
    void UART1_SendString(u8* Data,u16 len)
    {
      u16 i=0;
      for(;i<len;i++)
        UART1_SendByte(Data[i]);
    }
    /**********************************************************************************
     * 函数名：UART1_ReceiveByte
     * 描述  ：uart查询接收一个字节
     * 返回  ：一个字节 
     * 举例  ：temp=UART1_ReceiveByte()
     **********************************************************************************/
    u8 UART1_ReceiveByte(void)
    {
         u8 USART1_RX_BUF; 
         while (!(UART1_SR & UART1_FLAG_RXNE));
         USART1_RX_BUF=(uint8_t)UART1_DR;
         return  USART1_RX_BUF;
    }
    
    
    /***********************************************
     * 函数名：fputc
     * 描述  ：重定向c库函数printf到USART1
     * 输入  ：无
     * 输出  ：无
     * 调用  ：由printf调用
     ***********************************************/
    int fputc(int ch, FILE *f)
    {  
     /*将Printf内容发往串口*/ 
      UART1_DR=(unsigned char)ch;
      while (!(UART1_SR & UART1_FLAG_TXE));
      return (ch);
    }
    
    #pragma vector=0x14
    __interrupt void UART1_RX_IRQHandler(void)
    { 
        u8 Res;
        if(UART1_SR & UART1_FLAG_RXNE)  
        {/*接收中断(接收到的数据必须是0x0d 0x0a结尾)*/
            Res =(uint8_t)UART1_DR;
            /*(USART1->DR);读取接收到的数据,当读完数据后自动取消RXNE的中断标志位*/
            if(( UART_RX_NUM&0x80)==0)/*接收未完成*/
            {
                if( UART_RX_NUM&0x40)/*接收到了0x0d*/
                {
                    if(Res!=0x0a) UART_RX_NUM=0;/*接收错误,重新开始*/
                    else  UART_RX_NUM|=0x80;	/*接收完成了 */
                }
                else /*还没收到0X0D*/
                {	
                    if(Res==0x0d) UART_RX_NUM|=0x40;
                    else
                    {
                        RxBuffer[ UART_RX_NUM&0X3F]=Res ;
                        UART_RX_NUM++;
                        if( UART_RX_NUM>63) UART_RX_NUM=0;/*接收数据错误,重新开始接收*/  
                    }		 
                }
            }  		 
        }
    }
    
    
    void Clk_conf(void)
    {
     
      CLK_CKDIVR&= (uint8_t)(~0x18);/*时钟复位*/
      CLK_CKDIVR|= (uint8_t)0x00;/*设置时钟为内部16M高速时钟*/
      
    }
    
    
    extern u8 RxBuffer[RxBufferSize];
    extern u8 UART_RX_NUM;
    int main(void)
    {
      /* Infinite loop */
       u8 len ;
      /*设置内部高速时钟16M为主时钟*/ 
      Clk_conf();
      uart_conf();
      EnableInterrupt;
      printf("\r\n硬件平台为:%s\r\n","iCreate STM8 开发板");
      printf("\r\n%s\r\n","基于风驰例程寄存器版本V1.0.0");
      printf("\r\n作者：%s\r\n","风驰");
      printf("\r\n修改时间：%s\r\n","2012-6-13");
      printf("\r\n本例程测试方法：%s\r\n","在串口助手输入字符或者字符串必须要按下回车键，再点击发送");
      while(1)
      {
        /* 添加你的代码  */
          if(UART_RX_NUM&0x80)
          {
            len=UART_RX_NUM&0x3f;/*得到此次接收到的数据长度*/
            UART1_SendString("You sent the messages is:",sizeof("You sent the messages is"));
            UART1_SendString(RxBuffer,len);
            printf("\r\n得到此次接收到的数据长度:%dByte\r\n",len);
            UART_RX_NUM=0;
          }
    
      
      }
    }    
    
    
