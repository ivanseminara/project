; ModuleID = 'stack_protection.c'
target datalayout = "e-p:64:64:64-i1:8:16-i8:8:16-i16:16-i32:32-i64:64-f64:64-f128:128-n32:64"
target triple = "riscv"

@.str = private unnamed_addr constant [25 x i8] c"Successfully terminated\0A\00", align 1
@.str1 = private unnamed_addr constant [47 x i8] c"Should fail to write on writing element 16...\0A\00", align 1
@.str2 = private unnamed_addr constant [32 x i8] c"Filling array element %d of %d\0A\00", align 1
@.str3 = private unnamed_addr constant [69 x i8] c"Should have terminated by now! Trying to return to bogus address...\0A\00", align 1

; Function Attrs: nounwind
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  call void @test()
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([25 x i8]* @.str, i32 0, i32 0))
  ret i32 1
}

; Function Attrs: nounwind
define void @test() #0 {
entry:
  %arr = alloca [16 x i32], align 4
  %i = alloca i32, align 4
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([47 x i8]* @.str1, i32 0, i32 0))
  store i32 0, i32* %i, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 64
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32* %i, align 4
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([32 x i8]* @.str2, i32 0, i32 0), i32 %1, i32 16)
  %2 = load i32* %i, align 4
  %3 = load i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [16 x i32]* %arr, i32 0, i64 %idxprom
  store i32 %2, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([69 x i8]* @.str3, i32 0, i32 0))
  ret void
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
