// serial project serial.go
package serial

/*
extern int setup_uart(int fd, int n_speed, int n_bits, char parity, int n_stop);
extern int read_uart(int fd, char *rcv_buf, int buf_len, int timeo_us);
extern int read_uart_frame_aggregation(int fd, char *rcv_buf, int buf_len, int timeo_us, int aggr_us);
extern int write_uart(int fd, char *send_buf, int len);
*/
import "C"

import (
	"errors"
	"os"
	"time"
	"unsafe"
)

const (
	INFINITE = -1
)

type Uart struct {
	f *os.File
	c *Config
}

type Config struct {
	Port   string
	Baud   int
	Bits   int
	Parity byte
	Stop   int
}

func OpenPort(c *Config) (*Uart, error) {
	f, err := os.OpenFile(c.Port, os.O_RDWR, 0)
	if err != nil {
		return nil, err
	}

	ret := C.setup_uart(C.int(f.Fd()), C.int(c.Baud), C.int(c.Bits), C.char(c.Parity), C.int(c.Stop))
	if ret != 0 {
		return nil, errors.New("setup uart fail")
	}

	u := new(Uart)
	u.c = c
	u.f = f
	return u, nil
}

func (u *Uart) Close() error {
	return u.f.Close()
}

//frame aggregation in timeo_us microsecond
func (u *Uart) ReadAggregation(buf []byte, timeo time.Duration, aggr time.Duration) (int, error) {
	timeo_us := -1
	if timeo > 0 {
		timeo_us = int(timeo / 1000)
	}

	size := C.read_uart_frame_aggregation(
		C.int(u.f.Fd()),
		(*C.char)(unsafe.Pointer(&buf[0])),
		C.int(len(buf)),
		C.int(timeo_us),
		C.int(aggr/1000))
	if size >= 0 {
		buf[size] = 0
		return int(size), nil
	} else {
		return 0, errors.New("connection error")
	}
}

//read data from uart
func (u *Uart) Read(buf []byte, timeo time.Duration) (int, error) {
	timeo_us := -1
	if timeo > 0 {
		timeo_us = int(timeo / 1000)
	}

	size := C.read_uart(
		C.int(u.f.Fd()),
		(*C.char)(unsafe.Pointer(&buf[0])),
		C.int(len(buf)),
		C.int(timeo_us))
	if size >= 0 {
		buf[size] = 0
		return int(size), nil
	} else {
		return 0, errors.New("connection error")
	}
}

func (u *Uart) Write(buf []byte) (int, error) {
	sendLen := C.int(len(buf))
	size := C.write_uart(C.int(u.f.Fd()), (*C.char)(unsafe.Pointer(&buf[0])), sendLen)

	if size < 0 {
		return -1, errors.New("pipe error")
	} else if size < sendLen {
		return len(buf), errors.New("length error")
	}
	return int(size), nil
}
