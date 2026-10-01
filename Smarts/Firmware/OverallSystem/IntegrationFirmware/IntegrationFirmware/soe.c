/*
 * soe.c
 *
 * Created: 1/10/2026 1:47:59 pm
 *  Author: byhz780
 */ 

#include "soe.h"


float soe_calculate(float supercap_voltage)
{
	if (supercap_voltage <= 2.5f)
	{
		return 0.0f;
	}

	if (supercap_voltage >= 3.5f)
	{
		return 100.0f;
	}

	float numerator =
	(supercap_voltage * supercap_voltage)
	- (2.5f * 2.5f);

	float denominator =
	(3.5f * 3.5f)
	- (2.5f * 2.5f);

	return (numerator / denominator) * 100.0f;
}