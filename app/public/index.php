<?php
    session_start();
    require_once '../src/autoload.php';
    
    $msg = '';
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
?>
<!DOCTYPE html>
<html lang ="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
	<title>Osoprom</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>
    <div class="container">
	<div class="jumbotron" style="text-align:center" onselectstart="return false">
    <div class="row">
        <div class="col-sm-8">
            <h1 >Osoprom</h1>
            <br><h4>Engineering project management simplified.</h4>
        </div>
        <div class="col-sm-4">
            <form method="post">
            <div class="form-group">
                <label for="email" class="form-label">Email</label>
                <input type="text" class="form-control" id="email" name="email" value="edward.poole@organiz.com">
            </div>
            <div class="form-group">
                <label for="password" class="form-label">Password</label>
                <input type="text" class="form-control" id="password" name="password" value="hjbvirj84n">
            </div>
            <?php
                if ($msg != ''){
                    echo '<div class="alert alert-danger">';
                    echo $msg;
                    echo '</div>';
                }
            ?>
            <button type="submit" class="btn btn-primary">Login</button> 
            </form>
        </div>
    </div>
	</div>	
	</div>	
</body>
</html>
