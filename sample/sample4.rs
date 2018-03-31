var intA = 1
var intB = 1
import("test","TestLibrary.dll")

while(intA <= 5)
  while(intB <= 3)
      log(GetOps())
	  intB = intB + 1
  end
  log("value of intA " + intA)
  intA = intA + 1
  intB = 1
end

release("test")