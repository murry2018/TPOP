class Increment implements StaticIcmp {
    public int cmp(Integer n1, Integer n2)
    {
	int i1 = n1.intValue();
	int i2 = n2.intValue();
	if (i1 < i2)
	    return -1;
	else if (i1 > i2)
	    return 1;
	else
	    return 0;
    }
}
