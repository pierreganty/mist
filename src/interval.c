// vim:sw=4:ts=4
/*
   This file is part of mist.

   mist is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, 2004, Pierre Ganty
 */

#define LOCAL
#include <stdlib.h>
#include <stdio.h>
#include "interval.h"
#include "proc.h"
#include "error.h"



/*reserved values*/
#define stBegOfList     MaxInt16
#define stEndOfList     (-MaxInt16)

ISTInterval IST_beg_of_list = {stBegOfList, stBegOfList};
ISTInterval IST_end_of_list = {stEndOfList, stEndOfList};

inline boolean ist_greater_value(Value1, Value2)
	integer32 Value1, Value2;
{
	boolean result = false;
	if (Value1 == INFINITY ) {
		if (Value2 != INFINITY){
			result = true;
		}
	} else if (Value2 != INFINITY && Value1 > Value2) {
		result = true;
	}  
	return result;
}


inline boolean ist_greater_or_equal_value(Value1, Value2)
	integer32 Value1, Value2;
{
	boolean result = false;
	if (Value1 == INFINITY ) {
		result = true;
	} else if (Value2 != INFINITY && Value1 >= Value2) {
		result = true;
	}  
	return result;
}

inline boolean ist_less_value(Value1, Value2)
	integer32 Value1, Value2;
{
	boolean result = false;
	if (Value2 == INFINITY ) {
		if (Value1 != INFINITY){
			result = true;
		}
	} else if (Value1 != INFINITY && Value1 < Value2) {
		result = true;
	}  
	return result;
}

inline boolean ist_less_or_equal_value(Value1, Value2)
	integer32 Value1, Value2;
{
	boolean result = false;
	if (Value2 == INFINITY){
		result = true;
	} else if (Value1 != INFINITY  && Value1 <= Value2) {
		result = true;
	}
	return result;  
}


inline integer32 ist_add_value(Value1, Value2)
	integer32 Value1, Value2;
{
	integer32 result;
	if (Value1 == INFINITY || Value2 == INFINITY){
		result = INFINITY;
	} else {
		result = (Value1 + Value2);
	}
	return result;  
}


inline integer32 ist_sub_value(Value1, Value2)
	integer32 Value1, Value2;
	/*
	 * Only the first value can be equal to INFINTY
	 * \infty - \infty is an indetermination and -\infty
	 * is not allowed in our marking ...
	 */
{
	integer32 result;
	if (Value2 == INFINITY){
		err_quit("interval.c : ist_sub_value : Value2 can't be equal to infinite\n");
	}

	if (Value1 == INFINITY){
		result = INFINITY;
	} else {
		result = (Value1 - Value2);
	}
	return result;  
}

inline integer32 max(Value1, Value2)
	integer32 Value1, Value2;
{
	return (ist_greater_or_equal_value(Value1,Value2) ? Value1 : Value2);
}

inline integer32 min(Value1, Value2)
    integer32 Value1, Value2;
{
    return (ist_less_or_equal_value(Value1,Value2) ? Value1 : Value2);
}
/*
 * In the following, we define an order over interval.
 * This order is useful to insert elements in list
 *
 * First we look at the leftbound and we use rightbound
 * when leftbounds are equals
 */
inline boolean ist_greater_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1->Left > Value2->Left) {
			result = true;
		} else if (Value1->Left == Value2->Left && ist_greater_value(Value1->Right,Value2->Right)) {
			result = true;   
		}
	}
	return result;

}


inline boolean ist_greater_or_equal_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1->Left > Value2->Left) {
			result = true;
		} else if (Value1->Left == Value2->Left && ist_greater_or_equal_value(Value1->Right,Value2->Right)) {
			result = true;   
		}
	}
	return result;
}


inline boolean ist_less_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1->Left < Value2->Left) 
			result = true;
		else {
			if (Value1->Left == Value2->Left && ist_less_value(Value1->Right,Value2->Right))
				result = true;   
		}
	}
	return result;
}

inline boolean ist_less_or_equal_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1->Left < Value2->Left) {
			result = true;
		} else if (Value1->Left == Value2->Left && ist_less_or_equal_value(Value1->Right,Value2->Right)){
			result = true;   
		}	
	}
	return result;
}

inline boolean ist_equal_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1 == Value2)
			result = true;
		else {
			if (Value1->Left == Value2->Left && Value1->Right == Value2->Right)
				result = true;
		}
	}
	return result;
}


inline boolean ist_not_equal_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if (Value1 != Value2){
			if (Value1->Left != Value2->Left || Value1->Right != Value2->Right){
				result = true;
			}
		}

	}
	return result;
}

inline boolean ist_is_unbounded(Value)
	ISTInterval *Value;
{
	boolean result=false;
	if (Value != NULL){
		if (Value->Right == INFINITY)
			result=true;
	}
	return result;
}

inline void ist_assign_interval_to_interval(receiver,sender) 
	ISTInterval *receiver, *sender;
{
	if (receiver != NULL && sender != NULL) {
		receiver->Left = sender->Left;
		receiver->Right = sender->Right;
	}
}

inline void ist_assign_values_to_interval(receiver, leftbound, rightbound)
	ISTInterval *receiver;
	integer32 leftbound, rightbound;
{
	if (receiver != NULL) {
		receiver->Left = leftbound;
		receiver->Right = rightbound;
	}
}


inline ISTInterval* ist_build_interval(leftbound, rightbound) 
	integer32 leftbound, rightbound;
{
	ISTInterval* Result;
	Result = ist_new_info();
	Result->Left=leftbound;
	Result->Right=rightbound;
	return Result;
}

inline ISTInterval* ist_copy_interval(interv)
	ISTInterval *interv;
{
	ISTInterval *Result;
	if(interv != NULL) {
		Result = ist_new_info();
		Result->Left = interv->Left;
		Result->Right = interv->Right;
	}
	return Result;
}

inline boolean ist_include_interval(Value1, Value2)
	ISTInterval *Value1, *Value2;
	/* s2 \subset s1 */
{
	boolean result = false;
	if (Value1 != NULL && Value2 != NULL) {
		if ((Value1->Left <= Value2->Left) && ist_greater_or_equal_value(Value1->Right, Value2->Right))
			result = true;
	}
	return result;  
}

ISTInterval* ist_intersect_intervals(interv1, interv2)
	ISTInterval *interv1,* interv2;
{
	ISTInterval *Result = NULL;
	if (interv1 != NULL && interv2 != NULL){
		if (ist_less_or_equal_value(interv1->Left,interv2->Right) && ist_greater_or_equal_value(interv1->Right,interv2->Left) )  {
			Result = ist_build_interval((interv2->Left > interv1->Left) ? interv2->Left : interv1->Left,
					(ist_greater_value(interv2->Right, interv1->Right)) ? interv1->Right : interv2->Right);

		} else {
			if ( ist_less_or_equal_value(interv2->Left,interv1->Right) && ist_greater_or_equal_value(interv2->Right,interv1->Left) )  {
				Result = ist_build_interval((interv1->Left > interv2->Left) ? interv1->Left : interv2->Left,
						(ist_greater_value(interv1->Right,interv2->Right)) ? interv2->Right : interv1->Right);
			}
		}
	}
	return Result;
}


inline void ist_add_value_to_interval(interv,value)
	ISTInterval *interv;
	integer32 value;
{
	if (interv != NULL){
		interv->Left = interv->Left + value;
		interv->Right = ist_add_value(interv->Right,value);
	}
}

inline void ist_sub_value_to_interval(interv,value)
	ISTInterval *interv;
	integer32 value;
{
	if (interv != NULL){
		interv->Left = interv->Left - value;
		interv->Right = ist_sub_value(interv->Right,value);
	}
}

inline void ist_add_interval_to_interval(interv1,interv2)
	ISTInterval *interv1,*interv2;
{
	if (interv1 != NULL && interv2 != NULL){
		interv1->Left = interv1->Left + interv2->Left;
		interv1->Right = ist_add_value(interv1->Right,interv2->Right);
	}
}

inline void ist_sub_interval_to_interval(interv1,interv2)
	ISTInterval *interv1, *interv2;
{
	// Care about Bound < 0 and interv2->Right == INFINITY
	if (interv1 != NULL && interv2 != NULL){
		interv1->Left = interv1->Left - interv2->Left;
		if (interv2->Right != INFINITY){
			interv1->Right = ist_sub_value(interv1->Right,interv2->Right);
		}
	}
}

inline void ist_multiply_left_and_right_bound_by_value(interv,value)
	ISTInterval *interv;
	integer32 value;
{
	if (interv != NULL){
		interv->Left = interv->Left * value;
		interv->Right = ((interv->Right == INFINITY) ? INFINITY : (interv->Right * value));
	}
}

boolean ist_convex_union(interv1,interv2)
	ISTInterval *interv1, *interv2;
{
	boolean res;
	res = false;
	if (ist_include_interval(interv1,interv2)){
		res = true;
	} else if (interv1->Right == INFINITY && interv2->Right == INFINITY) {
		interv1->Left = (interv1->Left > interv2->Left) ? interv2->Left : interv1->Left; 
		res = true;
	} else if (interv1->Right != INFINITY && interv2->Right == INFINITY ){
		if (interv1->Right >= (interv2->Left-1)){
			interv1->Right = INFINITY;
			interv1->Left =  (interv1->Left > interv2->Left) ? interv2->Left : interv1->Left;
			res = true;
		}
	} else if (interv1->Right == INFINITY && interv2->Right != INFINITY){
		if (interv1->Left <= (interv2->Right+1)) {
			// For the right we change we know that we haven't inclusion
			interv1->Left =   interv2->Left;
			res = true;
		}
	} else if (interv1->Right >= (interv2->Left-1)){
		if (interv1->Left <= (interv2->Right+1)){
			interv1->Left =  (interv1->Left > interv2->Left) ? interv2->Left : interv1->Left;
			interv1->Right =  (interv1->Right > interv2->Right) ? interv1->Right : interv2->Right;
			res = true;
		}
	}
	return res;
}
