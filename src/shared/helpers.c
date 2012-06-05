char num_to_char(int num) {
    switch(num) {
    	case 1:	return 'A'; break;
    	case 2:	return 'B'; break;
    	case 3:	return 'C'; break;
    	case 4:	return 'D'; break;
    	case 5:	return 'E'; break;
    	case 6:	return 'F'; break;  
    	case 7:	return 'G'; break;
    	case 8:	return 'H'; break;  
    }
    return 0;
}

int char_to_num(char chr) {
    switch(chr) {
    	case 'a':
    	case 'A': return 1; break;
    	case 'b':
    	case 'B': return 2; break;
    	case 'c':
    	case 'C': return 3; break;
    	case 'd':
    	case 'D': return 4; break;
    	case 'e':
    	case 'E': return 5; break;
    	case 'f':
    	case 'F': return 6; break;
    	case 'g':
    	case 'G': return 7; break;
    	case 'h':
    	case 'H': return 8; break;    	
    }
    return -1;
}

