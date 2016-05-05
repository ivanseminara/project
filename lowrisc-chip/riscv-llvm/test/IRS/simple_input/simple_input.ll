; ModuleID = 'simple_input.c'
target datalayout = "e-p:64:64:64-i1:8:16-i8:8:16-i16:16-i32:32-i64:64-f64:64-f128:128-n32:64"
target triple = "riscv"

@.str = private unnamed_addr constant [12 x i8] c"Function 2\0A\00", align 1
@.str1 = private unnamed_addr constant [15 x i8] c"Insert value: \00", align 1
@.str2 = private unnamed_addr constant [11 x i8] c"value: %s\0A\00", align 1
@.str3 = private unnamed_addr constant [12 x i8] c"Function 3\0A\00", align 1

; Function Attrs: nounwind
define void @function1() #0 {
entry:
  %buffer = alloca [10 x i8], align 1
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str, i32 0, i32 0))
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str1, i32 0, i32 0))
  %arraydecay = getelementptr inbounds [10 x i8]* %buffer, i32 0, i32 0
  %call2 = call i8* @gets(i8* %arraydecay)
  %arraydecay3 = getelementptr inbounds [10 x i8]* %buffer, i32 0, i32 0
  %call4 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.str2, i32 0, i32 0), i8* %arraydecay3)
  ret void
}

declare i32 @printf(i8*, ...) #1

declare i8* @gets(i8*) #1

; Function Attrs: nounwind
define void @function2() #0 {
entry:
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([12 x i8]* @.str3, i32 0, i32 0))
  ret void
}

; Function Attrs: nounwind
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %fp2 = alloca void (...)*, align 8
  store i32 0, i32* %retval
  call void @function1()
  store void (...)* bitcast (void ()* @function2 to void (...)*), void (...)** %fp2, align 8
  %0 = load void (...)** %fp2, align 8
  %callee.knr.cast = bitcast void (...)* %0 to void ()*
  call void %callee.knr.cast()
  ret i32 0
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
