; ModuleID = 'simple_input-opt.bc'
target datalayout = "e-p:64:64:64-i1:8:16-i8:8:16-i16:16-i32:32-i64:64-f64:64-f128:128-n32:64"
target triple = "riscv"

@.str = private unnamed_addr constant [12 x i8] c"Function 2\0A\00", align 1
@.str1 = private unnamed_addr constant [15 x i8] c"Insert value: \00", align 1
@.str2 = private unnamed_addr constant [11 x i8] c"value: %s\0A\00", align 1
@.str3 = private unnamed_addr constant [12 x i8] c"Function 3\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()* }] [{ i32, void ()* } { i32 0, void ()* @__llvm_riscv_init_tagged_memory_csrs }]

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
  %cast_to_int8_pointer = bitcast void (...)** %fp2 to i8*
  call void @llvm.riscv.store.tagged(i64 ptrtoint (void ()* @function2 to i64), i64 4, i8* %cast_to_int8_pointer)
  %cast_to_int8_pointer1 = bitcast void (...)** %fp2 to i8*
  %0 = call { i64, i64 } @llvm.riscv.load.tagged(i8* %cast_to_int8_pointer1)
  %extract_tag = extractvalue { i64, i64 } %0, 1
  %extract_pointer = extractvalue { i64, i64 } %0, 0
  %1 = icmp ne i64 %extract_tag, 4
  br i1 %1, label %2, label %3

; <label>:2                                       ; preds = %entry
  call void @__llvm_riscv_check_tagged_failure()
  unreachable

; <label>:3                                       ; preds = %entry
  %4 = inttoptr i64 %extract_pointer to void (...)*
  %callee.knr.cast = bitcast void (...)* %4 to void ()*
  call void %callee.knr.cast()
  ret i32 0
}

define linkonce void @__llvm_riscv_init_tagged_memory_csrs() {
entry:
  call void @llvm.riscv.set.tm.trap.ld(i64 12)
  call void @llvm.riscv.set.tm.trap.sd(i64 10)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.riscv.set.tm.trap.ld(i64) #2

; Function Attrs: nounwind
declare void @llvm.riscv.set.tm.trap.sd(i64) #2

; Function Attrs: noreturn
define linkonce void @__llvm_riscv_check_tagged_failure() #3 {
entry:
  call void @llvm.trap()
  ret void
}

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #4

; Function Attrs: nounwind
declare void @llvm.riscv.store.tagged(i64, i64, i8* nocapture) #2

; Function Attrs: nounwind readonly
declare { i64, i64 } @llvm.riscv.load.tagged(i8* nocapture) #5

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noreturn }
attributes #4 = { noreturn nounwind }
attributes #5 = { nounwind readonly }
