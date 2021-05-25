// Copyright 2021 The IconVG Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package lowlevel

import (
	"bufio"
	"fmt"
	"io"
)

// Disassemble writes src's disassembly to w.
//
// See https://github.com/google/iconvg/blob/main/spec/iconvg-spec.md#example
// (look for the text "annotated disassembly") or test/data/*.ivg.disassembly
// for example output.
func Disassemble(w io.Writer, src []byte) error {
	// Calling disassemble will make lots of small writes. If w is an
	// io.ByteWriter then assume that it is already buffered. Otherwise, wrap w
	// with our own buffering.
	if _, ok := w.(io.ByteWriter); ok {
		return disassemble(w, src)
	}
	bw := bufio.NewWriter(w)
	err0 := disassemble(bw, src)
	err1 := bw.Flush()
	if err0 != nil {
		return err0
	}
	return err1
}

func disassemble(w io.Writer, src []byte) error {
	p := func(b []byte, format string, args ...interface{}) {
		const hex = "0123456789abcdef"
		var buf [14]byte
		for i := range buf {
			buf[i] = ' '
		}
		for i, x := range b {
			buf[3*i+0] = hex[x>>4]
			buf[3*i+1] = hex[x&0x0f]
		}
		w.Write(buf[:])
		fmt.Fprintf(w, format, args...)
	}
	return decode(nil, p, nil, false, src, nil)
}
