; ModuleID = 'intrinsics.c'
; target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
; target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define double @dot_f64(double* %a, double* %b, i32 %len) nounwind {
  %1 = alloca double*, align 8
  %2 = alloca double*, align 8
  %3 = alloca i32, align 4
  %result = alloca double, align 8
  %i = alloca i32, align 4
  store double* %a, double** %1, align 8
  store double* %b, double** %2, align 8
  store i32 %len, i32* %3, align 4
  store double 0.000000e+00, double* %result, align 8
  store i32 0, i32* %i, align 4
  br label %4

; <label>:4                                       ; preds = %22, %0
  %5 = load i32* %i, align 4
  %6 = load i32* %3, align 4
  %7 = icmp slt i32 %5, %6
  br i1 %7, label %8, label %25

; <label>:8                                       ; preds = %4
  %9 = load i32* %i, align 4
  %10 = sext i32 %9 to i64
  %11 = load double** %1, align 8
  %12 = getelementptr inbounds double* %11, i64 %10
  %13 = load double* %12, align 8
  %14 = load i32* %i, align 4
  %15 = sext i32 %14 to i64
  %16 = load double** %2, align 8
  %17 = getelementptr inbounds double* %16, i64 %15
  %18 = load double* %17, align 8
  %19 = fmul double %13, %18
  %20 = load double* %result, align 8
  %21 = fadd double %20, %19
  store double %21, double* %result, align 8
  br label %22

; <label>:22                                      ; preds = %8
  %23 = load i32* %i, align 4
  %24 = add nsw i32 %23, 1
  store i32 %24, i32* %i, align 4
  br label %4

; <label>:25                                      ; preds = %4
  %26 = load double* %result, align 8
  ret double %26
}

; Function Attrs: nounwind uwtable
define float @dot_f32(float* %a, float* %b, i32 %len) nounwind {
  %1 = alloca float*, align 8
  %2 = alloca float*, align 8
  %3 = alloca i32, align 4
  %result = alloca float, align 4
  %i = alloca i32, align 4
  store float* %a, float** %1, align 8
  store float* %b, float** %2, align 8
  store i32 %len, i32* %3, align 4
  store float 0.000000e+00, float* %result, align 4
  store i32 0, i32* %i, align 4
  br label %4

; <label>:4                                       ; preds = %22, %0
  %5 = load i32* %i, align 4
  %6 = load i32* %3, align 4
  %7 = icmp slt i32 %5, %6
  br i1 %7, label %8, label %25

; <label>:8                                       ; preds = %4
  %9 = load i32* %i, align 4
  %10 = sext i32 %9 to i64
  %11 = load float** %1, align 8
  %12 = getelementptr inbounds float* %11, i64 %10
  %13 = load float* %12, align 4
  %14 = load i32* %i, align 4
  %15 = sext i32 %14 to i64
  %16 = load float** %2, align 8
  %17 = getelementptr inbounds float* %16, i64 %15
  %18 = load float* %17, align 4
  %19 = fmul float %13, %18
  %20 = load float* %result, align 4
  %21 = fadd float %20, %19
  store float %21, float* %result, align 4
  br label %22

; <label>:22                                      ; preds = %8
  %23 = load i32* %i, align 4
  %24 = add nsw i32 %23, 1
  store i32 %24, i32* %i, align 4
  br label %4

; <label>:25                                      ; preds = %4
  %26 = load float* %result, align 4
  ret float %26
}

; Function Attrs: nounwind uwtable
define double @norm_f64(double* %a, i32 %len) nounwind {
  %1 = alloca double*, align 8
  %2 = alloca i32, align 4
  store double* %a, double** %1, align 8
  store i32 %len, i32* %2, align 4
  %3 = load double** %1, align 8
  %4 = load double** %1, align 8
  %5 = load i32* %2, align 4
  %6 = call double @dot_f64(double* %3, double* %4, i32 %5)
  %7 = fptrunc double %6 to float
  %8 = call float @__nv_sqrtf(float %7)
  %9 = fpext float %8 to double
  ret double %9
}

declare float @__nv_sqrtf(float)

; Function Attrs: nounwind uwtable
define float @norm_f32(float* %a, i32 %len) nounwind {
  %1 = alloca float*, align 8
  %2 = alloca i32, align 4
  store float* %a, float** %1, align 8
  store i32 %len, i32* %2, align 4
  %3 = load float** %1, align 8
  %4 = load float** %1, align 8
  %5 = load i32* %2, align 4
  %6 = call float @dot_f32(float* %3, float* %4, i32 %5)
  %7 = call float @__nv_sqrtf(float %6)
  ret float %7
}

; Function Attrs: nounwind uwtable
define double @det3_f64(double* %a) nounwind {
  %1 = alloca double*, align 8
  store double* %a, double** %1, align 8
  %2 = load double** %1, align 8
  %3 = getelementptr inbounds double* %2, i64 0
  %4 = load double* %3, align 8
  %5 = load double** %1, align 8
  %6 = getelementptr inbounds double* %5, i64 4
  %7 = load double* %6, align 8
  %8 = load double** %1, align 8
  %9 = getelementptr inbounds double* %8, i64 8
  %10 = load double* %9, align 8
  %11 = fmul double %7, %10
  %12 = load double** %1, align 8
  %13 = getelementptr inbounds double* %12, i64 5
  %14 = load double* %13, align 8
  %15 = load double** %1, align 8
  %16 = getelementptr inbounds double* %15, i64 7
  %17 = load double* %16, align 8
  %18 = fmul double %14, %17
  %19 = fsub double %11, %18
  %20 = fmul double %4, %19
  %21 = load double** %1, align 8
  %22 = getelementptr inbounds double* %21, i64 1
  %23 = load double* %22, align 8
  %24 = load double** %1, align 8
  %25 = getelementptr inbounds double* %24, i64 3
  %26 = load double* %25, align 8
  %27 = load double** %1, align 8
  %28 = getelementptr inbounds double* %27, i64 8
  %29 = load double* %28, align 8
  %30 = fmul double %26, %29
  %31 = load double** %1, align 8
  %32 = getelementptr inbounds double* %31, i64 5
  %33 = load double* %32, align 8
  %34 = load double** %1, align 8
  %35 = getelementptr inbounds double* %34, i64 6
  %36 = load double* %35, align 8
  %37 = fmul double %33, %36
  %38 = fsub double %30, %37
  %39 = fmul double %23, %38
  %40 = fsub double %20, %39
  %41 = load double** %1, align 8
  %42 = getelementptr inbounds double* %41, i64 2
  %43 = load double* %42, align 8
  %44 = load double** %1, align 8
  %45 = getelementptr inbounds double* %44, i64 3
  %46 = load double* %45, align 8
  %47 = load double** %1, align 8
  %48 = getelementptr inbounds double* %47, i64 7
  %49 = load double* %48, align 8
  %50 = fmul double %46, %49
  %51 = load double** %1, align 8
  %52 = getelementptr inbounds double* %51, i64 4
  %53 = load double* %52, align 8
  %54 = load double** %1, align 8
  %55 = getelementptr inbounds double* %54, i64 6
  %56 = load double* %55, align 8
  %57 = fmul double %53, %56
  %58 = fsub double %50, %57
  %59 = fmul double %43, %58
  %60 = fadd double %40, %59
  ret double %60
}

; Function Attrs: nounwind uwtable
define float @det3_f32(float* %a) nounwind {
  %1 = alloca float*, align 8
  store float* %a, float** %1, align 8
  %2 = load float** %1, align 8
  %3 = getelementptr inbounds float* %2, i64 0
  %4 = load float* %3, align 4
  %5 = load float** %1, align 8
  %6 = getelementptr inbounds float* %5, i64 4
  %7 = load float* %6, align 4
  %8 = load float** %1, align 8
  %9 = getelementptr inbounds float* %8, i64 8
  %10 = load float* %9, align 4
  %11 = fmul float %7, %10
  %12 = load float** %1, align 8
  %13 = getelementptr inbounds float* %12, i64 5
  %14 = load float* %13, align 4
  %15 = load float** %1, align 8
  %16 = getelementptr inbounds float* %15, i64 7
  %17 = load float* %16, align 4
  %18 = fmul float %14, %17
  %19 = fsub float %11, %18
  %20 = fmul float %4, %19
  %21 = load float** %1, align 8
  %22 = getelementptr inbounds float* %21, i64 1
  %23 = load float* %22, align 4
  %24 = load float** %1, align 8
  %25 = getelementptr inbounds float* %24, i64 3
  %26 = load float* %25, align 4
  %27 = load float** %1, align 8
  %28 = getelementptr inbounds float* %27, i64 8
  %29 = load float* %28, align 4
  %30 = fmul float %26, %29
  %31 = load float** %1, align 8
  %32 = getelementptr inbounds float* %31, i64 5
  %33 = load float* %32, align 4
  %34 = load float** %1, align 8
  %35 = getelementptr inbounds float* %34, i64 6
  %36 = load float* %35, align 4
  %37 = fmul float %33, %36
  %38 = fsub float %30, %37
  %39 = fmul float %23, %38
  %40 = fsub float %20, %39
  %41 = load float** %1, align 8
  %42 = getelementptr inbounds float* %41, i64 2
  %43 = load float* %42, align 4
  %44 = load float** %1, align 8
  %45 = getelementptr inbounds float* %44, i64 3
  %46 = load float* %45, align 4
  %47 = load float** %1, align 8
  %48 = getelementptr inbounds float* %47, i64 7
  %49 = load float* %48, align 4
  %50 = fmul float %46, %49
  %51 = load float** %1, align 8
  %52 = getelementptr inbounds float* %51, i64 4
  %53 = load float* %52, align 4
  %54 = load float** %1, align 8
  %55 = getelementptr inbounds float* %54, i64 6
  %56 = load float* %55, align 4
  %57 = fmul float %53, %56
  %58 = fsub float %50, %57
  %59 = fmul float %43, %58
  %60 = fadd float %40, %59
  ret float %60
}

; Function Attrs: nounwind uwtable
define void @inv3_f64(double* %a, double* %inv) nounwind {
  %1 = alloca double*, align 8
  %2 = alloca double*, align 8
  %cof00 = alloca double, align 8
  %cof01 = alloca double, align 8
  %cof02 = alloca double, align 8
  %cof10 = alloca double, align 8
  %cof11 = alloca double, align 8
  %cof12 = alloca double, align 8
  %cof20 = alloca double, align 8
  %cof21 = alloca double, align 8
  %cof22 = alloca double, align 8
  %determ = alloca double, align 8
  store double* %a, double** %1, align 8
  store double* %inv, double** %2, align 8
  %3 = load double** %1, align 8
  %4 = getelementptr inbounds double* %3, i64 4
  %5 = load double* %4, align 8
  %6 = load double** %1, align 8
  %7 = getelementptr inbounds double* %6, i64 8
  %8 = load double* %7, align 8
  %9 = fmul double %5, %8
  %10 = load double** %1, align 8
  %11 = getelementptr inbounds double* %10, i64 5
  %12 = load double* %11, align 8
  %13 = load double** %1, align 8
  %14 = getelementptr inbounds double* %13, i64 7
  %15 = load double* %14, align 8
  %16 = fmul double %12, %15
  %17 = fsub double %9, %16
  store double %17, double* %cof00, align 8
  %18 = load double** %1, align 8
  %19 = getelementptr inbounds double* %18, i64 3
  %20 = load double* %19, align 8
  %21 = fsub double -0.000000e+00, %20
  %22 = load double** %1, align 8
  %23 = getelementptr inbounds double* %22, i64 8
  %24 = load double* %23, align 8
  %25 = fmul double %21, %24
  %26 = load double** %1, align 8
  %27 = getelementptr inbounds double* %26, i64 5
  %28 = load double* %27, align 8
  %29 = load double** %1, align 8
  %30 = getelementptr inbounds double* %29, i64 6
  %31 = load double* %30, align 8
  %32 = fmul double %28, %31
  %33 = fsub double %25, %32
  store double %33, double* %cof01, align 8
  %34 = load double** %1, align 8
  %35 = getelementptr inbounds double* %34, i64 3
  %36 = load double* %35, align 8
  %37 = load double** %1, align 8
  %38 = getelementptr inbounds double* %37, i64 7
  %39 = load double* %38, align 8
  %40 = fmul double %36, %39
  %41 = load double** %1, align 8
  %42 = getelementptr inbounds double* %41, i64 4
  %43 = load double* %42, align 8
  %44 = load double** %1, align 8
  %45 = getelementptr inbounds double* %44, i64 6
  %46 = load double* %45, align 8
  %47 = fmul double %43, %46
  %48 = fsub double %40, %47
  store double %48, double* %cof02, align 8
  %49 = load double** %1, align 8
  %50 = getelementptr inbounds double* %49, i64 1
  %51 = load double* %50, align 8
  %52 = fsub double -0.000000e+00, %51
  %53 = load double** %1, align 8
  %54 = getelementptr inbounds double* %53, i64 8
  %55 = load double* %54, align 8
  %56 = fmul double %52, %55
  %57 = load double** %1, align 8
  %58 = getelementptr inbounds double* %57, i64 2
  %59 = load double* %58, align 8
  %60 = load double** %1, align 8
  %61 = getelementptr inbounds double* %60, i64 7
  %62 = load double* %61, align 8
  %63 = fmul double %59, %62
  %64 = fsub double %56, %63
  store double %64, double* %cof10, align 8
  %65 = load double** %1, align 8
  %66 = getelementptr inbounds double* %65, i64 0
  %67 = load double* %66, align 8
  %68 = load double** %1, align 8
  %69 = getelementptr inbounds double* %68, i64 8
  %70 = load double* %69, align 8
  %71 = fmul double %67, %70
  %72 = load double** %1, align 8
  %73 = getelementptr inbounds double* %72, i64 2
  %74 = load double* %73, align 8
  %75 = load double** %1, align 8
  %76 = getelementptr inbounds double* %75, i64 7
  %77 = load double* %76, align 8
  %78 = fmul double %74, %77
  %79 = fsub double %71, %78
  store double %79, double* %cof11, align 8
  %80 = load double** %1, align 8
  %81 = getelementptr inbounds double* %80, i64 0
  %82 = load double* %81, align 8
  %83 = fsub double -0.000000e+00, %82
  %84 = load double** %1, align 8
  %85 = getelementptr inbounds double* %84, i64 7
  %86 = load double* %85, align 8
  %87 = fmul double %83, %86
  %88 = load double** %1, align 8
  %89 = getelementptr inbounds double* %88, i64 1
  %90 = load double* %89, align 8
  %91 = load double** %1, align 8
  %92 = getelementptr inbounds double* %91, i64 6
  %93 = load double* %92, align 8
  %94 = fmul double %90, %93
  %95 = fsub double %87, %94
  store double %95, double* %cof12, align 8
  %96 = load double** %1, align 8
  %97 = getelementptr inbounds double* %96, i64 1
  %98 = load double* %97, align 8
  %99 = load double** %1, align 8
  %100 = getelementptr inbounds double* %99, i64 5
  %101 = load double* %100, align 8
  %102 = fmul double %98, %101
  %103 = load double** %1, align 8
  %104 = getelementptr inbounds double* %103, i64 2
  %105 = load double* %104, align 8
  %106 = load double** %1, align 8
  %107 = getelementptr inbounds double* %106, i64 4
  %108 = load double* %107, align 8
  %109 = fmul double %105, %108
  %110 = fsub double %102, %109
  store double %110, double* %cof20, align 8
  %111 = load double** %1, align 8
  %112 = getelementptr inbounds double* %111, i64 0
  %113 = load double* %112, align 8
  %114 = fsub double -0.000000e+00, %113
  %115 = load double** %1, align 8
  %116 = getelementptr inbounds double* %115, i64 5
  %117 = load double* %116, align 8
  %118 = fmul double %114, %117
  %119 = load double** %1, align 8
  %120 = getelementptr inbounds double* %119, i64 2
  %121 = load double* %120, align 8
  %122 = load double** %1, align 8
  %123 = getelementptr inbounds double* %122, i64 3
  %124 = load double* %123, align 8
  %125 = fmul double %121, %124
  %126 = fsub double %118, %125
  store double %126, double* %cof21, align 8
  %127 = load double** %1, align 8
  %128 = getelementptr inbounds double* %127, i64 0
  %129 = load double* %128, align 8
  %130 = load double** %1, align 8
  %131 = getelementptr inbounds double* %130, i64 4
  %132 = load double* %131, align 8
  %133 = fmul double %129, %132
  %134 = load double** %1, align 8
  %135 = getelementptr inbounds double* %134, i64 1
  %136 = load double* %135, align 8
  %137 = load double** %1, align 8
  %138 = getelementptr inbounds double* %137, i64 3
  %139 = load double* %138, align 8
  %140 = fmul double %136, %139
  %141 = fsub double %133, %140
  store double %141, double* %cof22, align 8
  %142 = load double** %1, align 8
  %143 = getelementptr inbounds double* %142, i64 0
  %144 = load double* %143, align 8
  %145 = load double* %cof00, align 8
  %146 = fmul double %144, %145
  %147 = load double** %1, align 8
  %148 = getelementptr inbounds double* %147, i64 1
  %149 = load double* %148, align 8
  %150 = load double* %cof01, align 8
  %151 = fmul double %149, %150
  %152 = fadd double %146, %151
  %153 = load double** %1, align 8
  %154 = getelementptr inbounds double* %153, i64 2
  %155 = load double* %154, align 8
  %156 = load double* %cof02, align 8
  %157 = fmul double %155, %156
  %158 = fadd double %152, %157
  store double %158, double* %determ, align 8
  %159 = load double* %determ, align 8
  %160 = fdiv double 1.000000e+00, %159
  store double %160, double* %determ, align 8
  %161 = load double* %cof00, align 8
  %162 = load double* %determ, align 8
  %163 = fmul double %161, %162
  %164 = load double** %2, align 8
  %165 = getelementptr inbounds double* %164, i64 0
  store double %163, double* %165, align 8
  %166 = load double* %cof10, align 8
  %167 = load double* %determ, align 8
  %168 = fmul double %166, %167
  %169 = load double** %2, align 8
  %170 = getelementptr inbounds double* %169, i64 1
  store double %168, double* %170, align 8
  %171 = load double* %cof20, align 8
  %172 = load double* %determ, align 8
  %173 = fmul double %171, %172
  %174 = load double** %2, align 8
  %175 = getelementptr inbounds double* %174, i64 2
  store double %173, double* %175, align 8
  %176 = load double* %cof01, align 8
  %177 = load double* %determ, align 8
  %178 = fmul double %176, %177
  %179 = load double** %2, align 8
  %180 = getelementptr inbounds double* %179, i64 3
  store double %178, double* %180, align 8
  %181 = load double* %cof11, align 8
  %182 = load double* %determ, align 8
  %183 = fmul double %181, %182
  %184 = load double** %2, align 8
  %185 = getelementptr inbounds double* %184, i64 4
  store double %183, double* %185, align 8
  %186 = load double* %cof21, align 8
  %187 = load double* %determ, align 8
  %188 = fmul double %186, %187
  %189 = load double** %2, align 8
  %190 = getelementptr inbounds double* %189, i64 5
  store double %188, double* %190, align 8
  %191 = load double* %cof02, align 8
  %192 = load double* %determ, align 8
  %193 = fmul double %191, %192
  %194 = load double** %2, align 8
  %195 = getelementptr inbounds double* %194, i64 6
  store double %193, double* %195, align 8
  %196 = load double* %cof12, align 8
  %197 = load double* %determ, align 8
  %198 = fmul double %196, %197
  %199 = load double** %2, align 8
  %200 = getelementptr inbounds double* %199, i64 7
  store double %198, double* %200, align 8
  %201 = load double* %cof22, align 8
  %202 = load double* %determ, align 8
  %203 = fmul double %201, %202
  %204 = load double** %2, align 8
  %205 = getelementptr inbounds double* %204, i64 8
  store double %203, double* %205, align 8
  ret void
}

; Function Attrs: nounwind uwtable
define void @inv3_f32(float* %a, float* %inv) nounwind {
  %1 = alloca float*, align 8
  %2 = alloca float*, align 8
  %cof00 = alloca float, align 4
  %cof01 = alloca float, align 4
  %cof02 = alloca float, align 4
  %cof10 = alloca float, align 4
  %cof11 = alloca float, align 4
  %cof12 = alloca float, align 4
  %cof20 = alloca float, align 4
  %cof21 = alloca float, align 4
  %cof22 = alloca float, align 4
  %determ = alloca float, align 4
  store float* %a, float** %1, align 8
  store float* %inv, float** %2, align 8
  %3 = load float** %1, align 8
  %4 = getelementptr inbounds float* %3, i64 4
  %5 = load float* %4, align 4
  %6 = load float** %1, align 8
  %7 = getelementptr inbounds float* %6, i64 8
  %8 = load float* %7, align 4
  %9 = fmul float %5, %8
  %10 = load float** %1, align 8
  %11 = getelementptr inbounds float* %10, i64 5
  %12 = load float* %11, align 4
  %13 = load float** %1, align 8
  %14 = getelementptr inbounds float* %13, i64 7
  %15 = load float* %14, align 4
  %16 = fmul float %12, %15
  %17 = fsub float %9, %16
  store float %17, float* %cof00, align 4
  %18 = load float** %1, align 8
  %19 = getelementptr inbounds float* %18, i64 3
  %20 = load float* %19, align 4
  %21 = fsub float -0.000000e+00, %20
  %22 = load float** %1, align 8
  %23 = getelementptr inbounds float* %22, i64 8
  %24 = load float* %23, align 4
  %25 = fmul float %21, %24
  %26 = load float** %1, align 8
  %27 = getelementptr inbounds float* %26, i64 5
  %28 = load float* %27, align 4
  %29 = load float** %1, align 8
  %30 = getelementptr inbounds float* %29, i64 6
  %31 = load float* %30, align 4
  %32 = fmul float %28, %31
  %33 = fsub float %25, %32
  store float %33, float* %cof01, align 4
  %34 = load float** %1, align 8
  %35 = getelementptr inbounds float* %34, i64 3
  %36 = load float* %35, align 4
  %37 = load float** %1, align 8
  %38 = getelementptr inbounds float* %37, i64 7
  %39 = load float* %38, align 4
  %40 = fmul float %36, %39
  %41 = load float** %1, align 8
  %42 = getelementptr inbounds float* %41, i64 4
  %43 = load float* %42, align 4
  %44 = load float** %1, align 8
  %45 = getelementptr inbounds float* %44, i64 6
  %46 = load float* %45, align 4
  %47 = fmul float %43, %46
  %48 = fsub float %40, %47
  store float %48, float* %cof02, align 4
  %49 = load float** %1, align 8
  %50 = getelementptr inbounds float* %49, i64 1
  %51 = load float* %50, align 4
  %52 = fsub float -0.000000e+00, %51
  %53 = load float** %1, align 8
  %54 = getelementptr inbounds float* %53, i64 8
  %55 = load float* %54, align 4
  %56 = fmul float %52, %55
  %57 = load float** %1, align 8
  %58 = getelementptr inbounds float* %57, i64 2
  %59 = load float* %58, align 4
  %60 = load float** %1, align 8
  %61 = getelementptr inbounds float* %60, i64 7
  %62 = load float* %61, align 4
  %63 = fmul float %59, %62
  %64 = fsub float %56, %63
  store float %64, float* %cof10, align 4
  %65 = load float** %1, align 8
  %66 = getelementptr inbounds float* %65, i64 0
  %67 = load float* %66, align 4
  %68 = load float** %1, align 8
  %69 = getelementptr inbounds float* %68, i64 8
  %70 = load float* %69, align 4
  %71 = fmul float %67, %70
  %72 = load float** %1, align 8
  %73 = getelementptr inbounds float* %72, i64 2
  %74 = load float* %73, align 4
  %75 = load float** %1, align 8
  %76 = getelementptr inbounds float* %75, i64 7
  %77 = load float* %76, align 4
  %78 = fmul float %74, %77
  %79 = fsub float %71, %78
  store float %79, float* %cof11, align 4
  %80 = load float** %1, align 8
  %81 = getelementptr inbounds float* %80, i64 0
  %82 = load float* %81, align 4
  %83 = fsub float -0.000000e+00, %82
  %84 = load float** %1, align 8
  %85 = getelementptr inbounds float* %84, i64 7
  %86 = load float* %85, align 4
  %87 = fmul float %83, %86
  %88 = load float** %1, align 8
  %89 = getelementptr inbounds float* %88, i64 1
  %90 = load float* %89, align 4
  %91 = load float** %1, align 8
  %92 = getelementptr inbounds float* %91, i64 6
  %93 = load float* %92, align 4
  %94 = fmul float %90, %93
  %95 = fsub float %87, %94
  store float %95, float* %cof12, align 4
  %96 = load float** %1, align 8
  %97 = getelementptr inbounds float* %96, i64 1
  %98 = load float* %97, align 4
  %99 = load float** %1, align 8
  %100 = getelementptr inbounds float* %99, i64 5
  %101 = load float* %100, align 4
  %102 = fmul float %98, %101
  %103 = load float** %1, align 8
  %104 = getelementptr inbounds float* %103, i64 2
  %105 = load float* %104, align 4
  %106 = load float** %1, align 8
  %107 = getelementptr inbounds float* %106, i64 4
  %108 = load float* %107, align 4
  %109 = fmul float %105, %108
  %110 = fsub float %102, %109
  store float %110, float* %cof20, align 4
  %111 = load float** %1, align 8
  %112 = getelementptr inbounds float* %111, i64 0
  %113 = load float* %112, align 4
  %114 = fsub float -0.000000e+00, %113
  %115 = load float** %1, align 8
  %116 = getelementptr inbounds float* %115, i64 5
  %117 = load float* %116, align 4
  %118 = fmul float %114, %117
  %119 = load float** %1, align 8
  %120 = getelementptr inbounds float* %119, i64 2
  %121 = load float* %120, align 4
  %122 = load float** %1, align 8
  %123 = getelementptr inbounds float* %122, i64 3
  %124 = load float* %123, align 4
  %125 = fmul float %121, %124
  %126 = fsub float %118, %125
  store float %126, float* %cof21, align 4
  %127 = load float** %1, align 8
  %128 = getelementptr inbounds float* %127, i64 0
  %129 = load float* %128, align 4
  %130 = load float** %1, align 8
  %131 = getelementptr inbounds float* %130, i64 4
  %132 = load float* %131, align 4
  %133 = fmul float %129, %132
  %134 = load float** %1, align 8
  %135 = getelementptr inbounds float* %134, i64 1
  %136 = load float* %135, align 4
  %137 = load float** %1, align 8
  %138 = getelementptr inbounds float* %137, i64 3
  %139 = load float* %138, align 4
  %140 = fmul float %136, %139
  %141 = fsub float %133, %140
  store float %141, float* %cof22, align 4
  %142 = load float** %1, align 8
  %143 = getelementptr inbounds float* %142, i64 0
  %144 = load float* %143, align 4
  %145 = load float* %cof00, align 4
  %146 = fmul float %144, %145
  %147 = load float** %1, align 8
  %148 = getelementptr inbounds float* %147, i64 1
  %149 = load float* %148, align 4
  %150 = load float* %cof01, align 4
  %151 = fmul float %149, %150
  %152 = fadd float %146, %151
  %153 = load float** %1, align 8
  %154 = getelementptr inbounds float* %153, i64 2
  %155 = load float* %154, align 4
  %156 = load float* %cof02, align 4
  %157 = fmul float %155, %156
  %158 = fadd float %152, %157
  store float %158, float* %determ, align 4
  %159 = load float* %determ, align 4
  %160 = fpext float %159 to double
  %161 = fdiv double 1.000000e+00, %160
  %162 = fptrunc double %161 to float
  store float %162, float* %determ, align 4
  %163 = load float* %cof00, align 4
  %164 = load float* %determ, align 4
  %165 = fmul float %163, %164
  %166 = load float** %2, align 8
  %167 = getelementptr inbounds float* %166, i64 0
  store float %165, float* %167, align 4
  %168 = load float* %cof10, align 4
  %169 = load float* %determ, align 4
  %170 = fmul float %168, %169
  %171 = load float** %2, align 8
  %172 = getelementptr inbounds float* %171, i64 1
  store float %170, float* %172, align 4
  %173 = load float* %cof20, align 4
  %174 = load float* %determ, align 4
  %175 = fmul float %173, %174
  %176 = load float** %2, align 8
  %177 = getelementptr inbounds float* %176, i64 2
  store float %175, float* %177, align 4
  %178 = load float* %cof01, align 4
  %179 = load float* %determ, align 4
  %180 = fmul float %178, %179
  %181 = load float** %2, align 8
  %182 = getelementptr inbounds float* %181, i64 3
  store float %180, float* %182, align 4
  %183 = load float* %cof11, align 4
  %184 = load float* %determ, align 4
  %185 = fmul float %183, %184
  %186 = load float** %2, align 8
  %187 = getelementptr inbounds float* %186, i64 4
  store float %185, float* %187, align 4
  %188 = load float* %cof21, align 4
  %189 = load float* %determ, align 4
  %190 = fmul float %188, %189
  %191 = load float** %2, align 8
  %192 = getelementptr inbounds float* %191, i64 5
  store float %190, float* %192, align 4
  %193 = load float* %cof02, align 4
  %194 = load float* %determ, align 4
  %195 = fmul float %193, %194
  %196 = load float** %2, align 8
  %197 = getelementptr inbounds float* %196, i64 6
  store float %195, float* %197, align 4
  %198 = load float* %cof12, align 4
  %199 = load float* %determ, align 4
  %200 = fmul float %198, %199
  %201 = load float** %2, align 8
  %202 = getelementptr inbounds float* %201, i64 7
  store float %200, float* %202, align 4
  %203 = load float* %cof22, align 4
  %204 = load float* %determ, align 4
  %205 = fmul float %203, %204
  %206 = load float** %2, align 8
  %207 = getelementptr inbounds float* %206, i64 8
  store float %205, float* %207, align 4
  ret void
}

!llvm.ident = !{!0}

!0 = metadata !{metadata !"Ubuntu clang version 3.4-1ubuntu3 (tags/RELEASE_34/final) (based on LLVM 3.4)"}
