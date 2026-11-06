<?php
    session_start();
    require_once '../src/autoload.php';

    $msg = '';
    $email = '';
    if (isset($_POST['email']) && isset($_POST['password'])){
        $email = $_POST['email'];
        $password = $_POST['password'];

        $authService = new AuthService();
        $result = $authService->authenticate($email, $password);

        if ($result['success']) {
            $_SESSION['eid'] = $result['employeeid'];
            header('Location:dashboard.php');
            return;
        } else {
            $msg = $result['message'];
        }
    }

    $pageTitle = 'Sign in';
    include 'includes/head.php';
?>
<body class="login-page">
    <div class="card login-card">
        <div class="card-body p-4 p-sm-5">
            <div class="text-center mb-4">
                <span class="brand-mark mb-3">O</span>
                <h1 class="h4 mb-1">Sign in to Osoem</h1>
                <p class="text-body-secondary mb-0">Engineering project management simplified.</p>
            </div>

            <?php if ($msg != ''): ?>
                <div class="alert alert-danger py-2" role="alert">
                    <i class="bi bi-exclamation-circle me-1"></i> <?php echo htmlspecialchars($msg); ?>
                </div>
            <?php endif; ?>

            <form method="post">
                <div class="mb-3">
                    <label for="email" class="form-label">Email</label>
                    <input type="email" class="form-control" id="email" name="email" autocomplete="username"
                           value="<?php echo htmlspecialchars($email); ?>" required autofocus>
                </div>
                <div class="mb-4">
                    <label for="password" class="form-label">Password</label>
                    <input type="password" class="form-control" id="password" name="password" autocomplete="current-password" required>
                </div>
                <button type="submit" class="btn btn-primary w-100">Sign in</button>
            </form>

            <div class="mt-4 pt-3 border-top small text-body-secondary text-center">
                Trying it out?
                <button type="button" class="btn btn-link btn-sm p-0 align-baseline" id="useDemo">Use the demo account</button>
            </div>
        </div>
    </div>

    <script>
    document.getElementById('useDemo').addEventListener('click', function () {
        document.getElementById('email').value = 'edward.poole@organiz.com';
        document.getElementById('password').value = 'hjbvirj84n';
    });
    </script>
</body>
</html>
