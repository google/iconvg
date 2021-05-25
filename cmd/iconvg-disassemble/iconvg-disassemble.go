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

// ----------------

// iconvg-disassemble prints a human-readable disassembly of IconVG byte-code.
//
// Usage: iconvg-disassemble in.ivg > out.ivg.disassembly
//     in.ivg may be omitted, in which case stdin is read.
package main

import (
	"fmt"
	"io"
	"os"

	"github.com/google/iconvg/src/go/lowlevel"
)

func main() {
	if err := main1(); err != nil {
		os.Stderr.WriteString(err.Error() + "\n")
		os.Exit(1)
	}
}

func main1() error {
	cmd := "iconvg-disassemble"
	if len(os.Args) > 0 {
		cmd = os.Args[0]
	}

	data := []byte(nil)
	in := os.Stdin
	if len(os.Args) > 2 {
		return fmt.Errorf("Usage: %s in.ivg > out.ivg.disassembly\n"+
			"    in.ivg may be omitted, in which case stdin is read.", cmd)
	} else if len(os.Args) == 2 {
		if f, err := os.Open(os.Args[1]); err != nil {
			return err
		} else {
			defer f.Close()
			in = f
		}
	}
	data, err := io.ReadAll(in)
	if err != nil {
		return err
	}

	return lowlevel.Disassemble(os.Stdout, data)
}
