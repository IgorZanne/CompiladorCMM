;ModuleID = '/tmp/webcompile/_5832_0.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


@.str = private unnamed_addr constant [4 x i8] c"%i\0A\00", align 1
define i32 @main() #0 {
entry:
%a = alloca i32, align 4
store i32 5, i32* %a, align 4
%b = alloca i32, align 4
store i32 10, i32* %b, align 4
%0 = load i32* %a, align 4
%1 = load i32* %b, align 4
%add = add i32 %0, %1
%c = alloca i32, align 4
store i32 %add, i32* %c, align 4
%2 = load i32* %c, align 4
%call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %2)

ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}
!0 = metadata !{metadata !"clang version 3.6.0 (trunk)"}