// Based on https://github.com/pfeerick/elapsedMillis
//
// Copyright (c) 2011 PJRC.COM, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Arduino.h"

class elapsed_millis
{
private:
	unsigned long ms;
public:
	elapsed_millis(void) { ms = millis(); }
	elapsed_millis(unsigned long val) { ms = millis() - val; }
	elapsed_millis(const elapsed_millis &orig) { ms = orig.ms; }
	operator unsigned long () const { return millis() - ms; }
	elapsed_millis & operator = (const elapsed_millis &rhs) { ms = rhs.ms; return *this; }
	elapsed_millis & operator = (unsigned long val) { ms = millis() - val; return *this; }
	elapsed_millis & operator -= (unsigned long val)      { ms += val ; return *this; }
	elapsed_millis & operator += (unsigned long val)      { ms -= val ; return *this; }
	elapsed_millis operator - (int val) const           { elapsed_millis r(*this); r.ms += val; return r; }
	elapsed_millis operator - (unsigned int val) const  { elapsed_millis r(*this); r.ms += val; return r; }
	elapsed_millis operator - (long val) const          { elapsed_millis r(*this); r.ms += val; return r; }
	elapsed_millis operator - (unsigned long val) const { elapsed_millis r(*this); r.ms += val; return r; }
	elapsed_millis operator + (int val) const           { elapsed_millis r(*this); r.ms -= val; return r; }
	elapsed_millis operator + (unsigned int val) const  { elapsed_millis r(*this); r.ms -= val; return r; }
	elapsed_millis operator + (long val) const          { elapsed_millis r(*this); r.ms -= val; return r; }
	elapsed_millis operator + (unsigned long val) const { elapsed_millis r(*this); r.ms -= val; return r; }
};
