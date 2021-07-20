
class SMA {
  float readings[n], total = 0;
  byte Pos = 0;
  public :
  float average (float reading) {
    total -= readings[Pos];         //subtract old reading from total
    readings[Pos] = reading;        //place new reading into the array
    total += readings[Pos];         //add the new reading to the total
    (Pos < n-1 ? Pos++ : Pos = 0);  //Incrament Pos until the end is reached
    return total / n;               //take the current average
  }
  SMA () {                          //Constrctor to clear/set reading array
    for (byte t = 0; t < n; t++) {
    readings[t] = 0;
    }
  }
}objects;
