# serial

Example:

``` 
func main() {
	config := serial.Config{
		Port:   "/dev/ttyUSB0",
		Baud:   115200,
		Bits:   8,
		Parity: 'N',
		Stop:   1}
	uart, err := serial.OpenPort(&config)
	if err != nil {
		fmt.Println("open error")
		return
	}

	sendLen, _ := uart.Write([]byte("hello"))
	fmt.Println(sendLen)

	buf := make([]byte, 4096)
	for {
		size, err := uart.ReadAggregation(buf, serial.INFINITE, 50*time.Millisecond)
		//size, err := uart.Read(buf, 100*time.Millisecond)
		if err != nil {
			fmt.Printf("%s\n", err)
			break
		} else {
			if size > 0 {
				fmt.Printf("%d-%s\n",size, buf)
			}
		}
	}
}
```

